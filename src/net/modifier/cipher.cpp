#include <pch.hpp>
#include <wise.kernel/net/modifier/cipher.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include <botan/botan.h>
#include <botan/aes.h>
#include <botan/hex.h>

namespace wise {
namespace kernel {

cipher::config cipher::cfg;

constexpr std::size_t BLOCK_SIZE = Botan::AES_128::BLOCK_SIZE;

/// Implementation note. 
/**
 * 암호화는 여러 언어, 플래폼 호환이 돼야 하고 강력할수록 디버깅이
 * 어려워 구현이 까다로운 편이다. 게임은 보호해야 할 사용자들이
 * 해킹을 시도하므로 암호화를 통해 보안을 유지하기 어려운 시스템이다.
 * 따라서, 세션 암호화를 지켜서 패킷을 분석하기 어렵도록 하는
 * 정도가 가장 실용적인 방법이다. 오히려 서버에서 모든 패킷을
 * 서버 상태에 따라 철저하게 검증하는 것이 가장 중요하다.
 *
 * .NET의 AES 구현을 확인하는 과정에서 ICryptoTransform을
 * 매번 생성하는 샘플 밖에 찾을 수 없었고, 실제 테스트를 해본 결과
 * 재사용할 때 문제가 발생했다. (이상하지만 확인하는 비용이 커서 멈춤)
 *
 * CBC 모드를 사용하고 IV를 매번 변경하며 최초 한번, 그리고 일정한 주기로
 * 키를 변경함으로써 세션의 암호화된 패킷을 보호하는 구현을 갖는다.
 *
 * 최초에는 고정된 키를 갖고 있어 암호화를 사용하는 안 쓰는 패킷을
 * 랜던 데이터를 포함하여 송수신해야 이후 패킷들을 보호할 수 있다.
 */

struct cipher_impl
{
	std::vector<uint8_t> key;
	std::vector<uint8_t> nonce;
	std::vector<uint8_t> data;
	std::size_t			 count = 0;

	Botan::Cipher_Dir	direction;

	std::unique_ptr<Botan::HashFunction> hasher;
	std::unique_ptr<Botan::Cipher_Mode> algo;

	cipher_impl(Botan::Cipher_Dir dir)
		: hasher(Botan::HashFunction::create("SHA1"))
		, algo(Botan::get_cipher_mode("AES-128/CBC/PKCS7", dir))
	{
		WISE_UNUSED(dir);
	}

	void start()
	{
		algo->clear();
		algo->set_key(key);
		algo->start(nonce);
	}

	void update_hash(resize_buffer& buf, std::size_t msg_pos, std::size_t msg_len);

	void update_nonce();

	void update_key();

	void complete();
};

cipher::cipher(std::size_t header_length)
	: header_length_(header_length)
{
}

cipher::~cipher()
{
}

modifier::result cipher::on_bind()
{
	WISE_ASSERT(!bound_);

	receiver_ = wise_unique<cipher_impl>(Botan::Cipher_Dir::DECRYPTION);
	sender_ = wise_unique<cipher_impl>(Botan::Cipher_Dir::ENCRYPTION);

	receiver_->key = Botan::hex_decode("2B7E151628AED2A6ABF7158809CF4F3C");
	sender_->key = Botan::hex_decode("2B7E151628AED2A6ABF7158809CF4F3C");

	receiver_->nonce = Botan::hex_decode("ACE03D519F3CBA2BF67CF1B7E1C4F02D");
	sender_->nonce = Botan::hex_decode("ACE03D519F3CBA2BF67CF1B7E1C4F02D");

	receiver_->start();
	sender_->start();

	return result(true, reason::success);
}

modifier::result cipher::on_recv(
	resize_buffer& buf,
	std::size_t msg_pos,
	std::size_t msg_len,
	std::size_t& new_len
)
{
	new_len = msg_len;

	// header를 제외하고 암호화 
	WISE_ASSERT(msg_len >= header_length_);

	auto payload_size = msg_len - header_length_;
	WISE_RETURN_IF(payload_size == 0, result(true, reason::success)); // no data 

	// 암호화 할 때 블럭에 맞춤
	WISE_ASSERT(payload_size % BLOCK_SIZE == 0);

	auto cipher_len = msg_len - header_length_;
	auto cipher_pos = msg_pos + header_length_;
	auto final_pos = cipher_pos + cipher_len - BLOCK_SIZE;	// 마지막 블럭은 패딩 때문에 별도로 처리 (final 함수)
	uint8_t* cipher_ptr = buf.data() + cipher_pos;

	std::size_t pad_size = 0;

	// 세션 별로 여기를 호출하는 쓰레드는 한번에 하나 밖에 없다.

	// 매번 재시작. IV만 교체. 주기적으로 Key 교체
	receiver_->start();

	// decryption
	{
		std::size_t process_len = cipher_len - BLOCK_SIZE;

		if (process_len > 0)
		{
			receiver_->algo->process(cipher_ptr, process_len);
		}

		Botan::secure_vector<uint8_t> final_block;
		final_block.reserve(BLOCK_SIZE);

		std::copy(
			buf.begin() + (ptrdiff_t)final_pos,
			buf.begin() + (ptrdiff_t)(final_pos + BLOCK_SIZE),
			std::back_inserter(final_block)
		);

		receiver_->algo->finish(final_block);

		std::memcpy(
			buf.data() + final_pos,
			final_block.data(),
			final_block.size()
		);

		pad_size = BLOCK_SIZE - final_block.size();
	}

	// 락 불필요 (한번에 하나씩 처리됨)
	{
		receiver_->update_hash(
			buf,
			msg_pos + header_length_,
			msg_len - header_length_ - pad_size
		);

		receiver_->complete();
	}

	new_len = msg_len - pad_size;

	return result(true, reason::success);
}

modifier::result cipher::on_send(
	resize_buffer& buf,
	std::size_t msg_pos,
	std::size_t msg_len
)
{
	// header를 제외하고 암호화 
	WISE_ASSERT(msg_len >= header_length_);

	auto payload_size = msg_len - header_length_;
	WISE_RETURN_IF(payload_size == 0, result(true, reason::success)); // no data 

	auto pad_size = BLOCK_SIZE - payload_size % BLOCK_SIZE;

	const auto pad = Botan::hex_decode("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
	buf.append(pad.data(), pad_size);

	auto cipher_len = payload_size + pad_size;					// 암호화 길이
	auto cipher_pos = msg_pos + header_length_;	// 암호화 시작 위치
	auto final_pos = cipher_pos + cipher_len - BLOCK_SIZE;		// 최종 블럭 위치	
	uint8_t* cipher_ptr = buf.data() + cipher_pos;					// 암호화 시작 포인터

	// 세션 별로 여기를 호출하는 쓰레드는 한번에 하나 밖에 없다.

	// 매번 재시작. IV만 교체. 주기적으로 Key 교체
	sender_->start();

	sender_->update_hash(
		buf,
		msg_pos + header_length_,
		msg_len - header_length_
	);

	// encryption 
	{
		std::size_t process_len = cipher_len - BLOCK_SIZE;

		if (process_len > 0)
		{
			sender_->algo->process(cipher_ptr, process_len);
		}

		Botan::secure_vector<uint8_t> final_block;
		final_block.reserve(BLOCK_SIZE);

		std::copy(
			buf.begin() + (ptrdiff_t)final_pos,
			buf.begin() + (ptrdiff_t)(final_pos + BLOCK_SIZE - pad_size),
			std::back_inserter(final_block)
		);

		sender_->algo->finish(final_block);

		WISE_ASSERT(final_block.size() == BLOCK_SIZE);

		std::memcpy(
			buf.data() + final_pos,
			final_block.data(),
			final_block.size()
		);
	}

	sender_->complete();

	update_length_field(buf, msg_pos, msg_len + pad_size);

	return result(true, reason::success);
}

void cipher_impl::update_hash(
	resize_buffer& buf, std::size_t msg_pos, std::size_t msg_len)
{
	const uint8_t* pd = buf.data() + msg_pos; // msg_pos가 header 길이를 반영 바뀌어 옴

	auto len = std::min<std::size_t>(msg_len, 64);

	hasher->update(pd, len);
	hasher->update(data);
	hasher->final(data);

	// 성능 보다 보안이 더 중요하여 CRC32대신 SHA1을 사용한다. 
}

void cipher_impl::complete()
{
	// count가 0일 때도 변경
	if (count % cipher::cfg.key_change_interval == 0)
	{
		update_key();
	}

	update_nonce();

	count++;
}

void cipher_impl::update_nonce()
{
	nonce.clear();
	std::copy(data.begin(), data.begin() + 16, std::back_inserter(nonce));
}

void cipher_impl::update_key()
{
	key.clear();
	std::copy(data.begin(), data.begin() + 16, std::back_inserter(key));
}

} // kernel
} // wise
