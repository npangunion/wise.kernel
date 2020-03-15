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
 * ��ȣȭ�� ���� ���, �÷��� ȣȯ�� �ž� �ϰ� �����Ҽ��� �������
 * ����� ������ ��ٷο� ���̴�. ������ ��ȣ�ؾ� �� ����ڵ���
 * ��ŷ�� �õ��ϹǷ� ��ȣȭ�� ���� ������ �����ϱ� ����� �ý����̴�.
 * ����, ���� ��ȣȭ�� ���Ѽ� ��Ŷ�� �м��ϱ� ��Ƶ��� �ϴ�
 * ������ ���� �ǿ����� ����̴�. ������ �������� ��� ��Ŷ��
 * ���� ���¿� ���� ö���ϰ� �����ϴ� ���� ���� �߿��ϴ�.
 *
 * .NET�� AES ������ Ȯ���ϴ� �������� ICryptoTransform��
 * �Ź� �����ϴ� ���� �ۿ� ã�� �� ������, ���� �׽�Ʈ�� �غ� ���
 * ������ �� ������ �߻��ߴ�. (�̻������� Ȯ���ϴ� ����� Ŀ�� ����)
 *
 * CBC ��带 ����ϰ� IV�� �Ź� �����ϸ� ���� �ѹ�, �׸��� ������ �ֱ��
 * Ű�� ���������ν� ������ ��ȣȭ�� ��Ŷ�� ��ȣ�ϴ� ������ ���´�.
 *
 * ���ʿ��� ������ Ű�� ���� �־� ��ȣȭ�� ����ϴ� �� ���� ��Ŷ��
 * ���� �����͸� �����Ͽ� �ۼ����ؾ� ���� ��Ŷ���� ��ȣ�� �� �ִ�.
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

	// header�� �����ϰ� ��ȣȭ 
	WISE_ASSERT(msg_len >= header_length_);

	auto payload_size = msg_len - header_length_;
	WISE_RETURN_IF(payload_size == 0, result(true, reason::success)); // no data 

	// ��ȣȭ �� �� ���� ����
	WISE_ASSERT(payload_size % BLOCK_SIZE == 0);

	auto cipher_len = msg_len - header_length_;
	auto cipher_pos = msg_pos + header_length_;
	auto final_pos = cipher_pos + cipher_len - BLOCK_SIZE;	// ������ ���� �е� ������ ������ ó�� (final �Լ�)
	uint8_t* cipher_ptr = buf.data() + cipher_pos;

	std::size_t pad_size = 0;

	// ���� ���� ���⸦ ȣ���ϴ� ������� �ѹ��� �ϳ� �ۿ� ����.

	// �Ź� �����. IV�� ��ü. �ֱ������� Key ��ü
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

	// �� ���ʿ� (�ѹ��� �ϳ��� ó����)
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
	// header�� �����ϰ� ��ȣȭ 
	WISE_ASSERT(msg_len >= header_length_);

	auto payload_size = msg_len - header_length_;
	WISE_RETURN_IF(payload_size == 0, result(true, reason::success)); // no data 

	auto pad_size = BLOCK_SIZE - payload_size % BLOCK_SIZE;

	const auto pad = Botan::hex_decode("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
	buf.append(pad.data(), pad_size);

	auto cipher_len = payload_size + pad_size;					// ��ȣȭ ����
	auto cipher_pos = msg_pos + header_length_;	// ��ȣȭ ���� ��ġ
	auto final_pos = cipher_pos + cipher_len - BLOCK_SIZE;		// ���� �� ��ġ	
	uint8_t* cipher_ptr = buf.data() + cipher_pos;					// ��ȣȭ ���� ������

	// ���� ���� ���⸦ ȣ���ϴ� ������� �ѹ��� �ϳ� �ۿ� ����.

	// �Ź� �����. IV�� ��ü. �ֱ������� Key ��ü
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
	const uint8_t* pd = buf.data() + msg_pos; // msg_pos�� header ���̸� �ݿ� �ٲ�� ��

	auto len = std::min<std::size_t>(msg_len, 64);

	hasher->update(pd, len);
	hasher->update(data);
	hasher->final(data);

	// ���� ���� ������ �� �߿��Ͽ� CRC32��� SHA1�� ����Ѵ�. 
}

void cipher_impl::complete()
{
	// count�� 0�� ���� ����
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
