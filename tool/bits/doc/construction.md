# Construction  



# tx 확장 

## 테스트 

테스트를 많이 해야 한다. 깔끔하고 쉬워야 하고 안정적이어야 한다.  잘못된 부분이 있으면 수 많은 사람이 수 없이 고생하게 된다. 아무리 강조해도 지나치지 않다. 

db_service와 클라이언트를 만들고 테스트 한다. 디스패칭을 포함하여 테스트 한다.  첫 서비스이므로 다른 부분 개선을 포함한다. 10월 안에는 꼭 끝내도록 한다. 

- db_service를 통한 실행 
  - 메세지로 요청하고 받도록 하여 테스트 진행 
- 타잎들 지원 
  - string / wstring / binary / guid
  - 메세지 수준 
  - 쿼리 처리 
- 디버깅 개선 



## 코드 생성 

- tx::execute 



### struct, vec\<struct\> 형태의 바인딩 지원 (값 지정). 

- pack / unpack 
- execute 코드 

DB 작업을 효율적으로 처리하기 위해 꼭 필요하다.  코드 생성을 위해서도 필요하다. 

out으로는 지원하지 않는다. 많이 복잡해진다. result set으로 모두 처리 가능하다. 

- 심벌 테이블에 등록하기 OK
- vec<type, count> 지원 OK

이제 코드 생성으로. 중요한 건 에러 / 예외 사항의 처리다. 



### 정리 

완전하지는 않지만 꽤 많이 작성했다. 이제 테스트를 통해 완결해야 한다. 



## 개선 작업 



### dbc에서 string과 wstring의 동시 지원

string과 wstring을 모두 처리 가능하게 한다. 실제 varchar, nvarchar는 동시에 사용하는 경우가 많다. 



### dbc에서 바이너리 / GUID 필드 지원 

길드 마크 이미지 등에 사용한다. 지원이 있어야 한다. 필드 타잎에서 먼저 할 지 나중에 할 지는 따로 생각한다. 



### null 값 처리 

실제 null 값은 게임에서 사용하지 않는다. 여러 이슈가 발생할 수 있기 때문이다. 



### sql 생성 

프러시져 아규먼트 템플릿 생성. 이건 기본 기능으로 갖도록 한다. 

- 간단하게 확인 가능한 수준으로 생성한다. 
- 이 정도 힌트만 있어도 많은 도움이 된다. 



## 간단한 쿼리 



```c++
namespace query; 

/**
 * A very simple query to test a simple generation
 */
tx simple_query 
{    

   topic db.query.simple;
    
   bind 
   {
       u32 result : out;
       string name; 
   }
    
    result 
	{
        single 
		{
            u32 id; 
        }
	}
}
```

가장 단순한 기능이다. 구현을 먼저 하고 테스트를 하나씩 하는 방식으로 진행한다. 

### 키워드 

- tx
- tx_bind 
- tx_result 
- tx_single 
- tx_multi 



tx가 없으면 너무 일반적인 이름이 키워드가 된다. 따라서, tx를 붙여서 작업하도록 한다.  많이 이상해 보이지는 않고 익숙해지면 더 깔끔해 보일 것으로 예상한다. 문맥에 따라 키워드를 다르게 하는 기능을 지원하기는 flex, bison으로는 어렵다. 



### 노드 추가 

- idl_node_tx

- tx_topic
- tx_bind 
- tx_result
- tx_result_set
  - single / multi 



여기까지 큰 장애물은 없었다. 빈 규칙으로 재귀를 설정하는 부분이랑 몇 가지 실수들. 5시간 정도 걸림. 



### 필드 

- out attribute 
- size?



이 부분 중요. 어떻게 일관되게 할 지 생각하고 진행. 

string<len> 

ustring<len> 



결과 집합은 길이 지정이 필요 없다.  호출 시 바인딩 시 출력 메모리 할당은 필요하다. 이를 위해 길이 값이 있어야 한다. 문자열 출력에 대해 메모리 관련 처리가 필요하다.  레전드에서는 SqlBinaryData로 템플릿을 만들고 여기에 있는 메모리로 바인딩을 한다.  Get / Set으로 필요한 곳에 넘긴다. 

지원해야 할 지는 아직 잘 모르겠다. 특별히 용도가 없어 보이기도 한다. Result Set이 훨씬 효과적이다.  코드 생성 작업을 하고 나서 살핀다.





## 작업 

- 키워드 
  - tx, bind, result, single, multi 
- 타잎 
  - 길이 추가 
    - string, vec 





## 예시. 전체 기능 

 ```c++

tx get_character {    
   topic db.query.get_character; 
    
   tx_bind {
       u32 result : out;
       string name; 
       string<20> aa : out; // 명시적인 길이 지정이 필요하다. reserve를 해서 bind 한다. 
   }
    
   tx_result {
       tx_single {        
            u32 id; 
            info fo;
       } 
       
       tx_multi { 
       		string name;
       }
   }             
}

 ```



- bind에 SP의 파라미터 문법에 가깝게 정의 
  - 개별 항목 정의 방식은 유지 
- result는 single, multi 로 지정 
  - 멀티만 vec 안에 structure를 넣도록 함 
  - 필드생성 
    - set1, set2, ... 



과제들이 꽤 있지만 한 달 정도 안정화 시키고 개선하면 꽤 괜찮을 것 같다. 



### 실제 게임 예제 

캐릭터 정보 저장하는 SP 호출을 위한 파라미터 목록이다. 

```c++
	SqlInt		result, charId, accountId, health, mana, zoneId, channelId, partyId, colosseumPoint, colosseumVictoryCount, colosseumDefeatCount;
	SqlTinyInt	grade, level, making1stJobIndex, making1stJobGrade, making2ndJobIndex, making2ndJobGrade, achieveGrade, colosseumCWCount, colosseumTitleId;
	SqlSmallInt		soulLevel, soulPoint, titleId;
	SqlBigInt	exp, total_exp,/* money,*/ soulExp, total_soulExp, currentContinent;
	SqlFloat	x, y, z;
	SqlInt		achievePoint, making1stJobExp, making2ndJobExp, petCombatPower;
	SqlInt      footholdIndex, tutorialState;
	SqlTinyInt	currentSkillQuickSlotPage;
	SqlInt		combatPower;
	SqlBigInt	contributionPoint, greenZen, riftPoint, maxRiftPoint;
	SqlInt		eventInventoryLastCheckIndex;
	SqlBigInt	wShopInventoryLastCheckTime;
```

in의 구조체 내용이다.

```c++
tx save_character { 

    sp UspSaveCharacter; 
    
    bind { 
        u32 result : out, charId, accountId, health, mana, zoneId, channelId, partyId; 
        u32 colosseumPoint, colosseumVictoryCount, colosseumDefeatCount;
        u8 grade, level, making1stJobIndex, making1stJobGrade, making2ndJobIndex; 
        u8 making2ndJobGrade, achieveGrade, colosseumCWCount, colosseumTitleId;
        u16 soulLevel, soulPoint, titleId;
        u64	exp, total_exp,/* money,*/ soulExp, total_soulExp, currentContinent;
        float x, y, z;
        // ... 
    }
}
```

순서는 쓰는 대로 하고 방향은 뒤에 붙이는 것으로 한다. 항목이 많으므로 한 줄에 여러 개를 쓸 수 있게 한다. 

구조체를 쓸 수 있게 한다. 



# 작업 

- c++ 코드 생성 및 테스트 
  - 파서 작성 
  - 코드 생성 
  - 위를 enum, struct, message와 타잎들에 대해 반복 
  - include 처리 
  - namespace 처리 
  - 디버깅 기능 추가




아래 내용은 작업 히스토리로 역순으로 정리한다. 

- 테스트를 통한 완성이 중요하다. 
- 편하게 안심하고 쓸 수 있어야 한다. 
- 확장과 개선이 쉬워야 한다. 




## 옵션 설정

json으로 여러 가지 규칙을 정한다.  

- 파일 확장자 
  - 헤더 파일 
- 변수 prefix (접두어)



## 테스트 

실제 게임 이벤트들로 작성하고 확인. 이제 샘플 프로젝트가 필요하다. C++로 한참을 가고 난 후에 C# 생성할 때 다시 돌아와 정리한다.

- 에러 처리 
  - 파싱 에러 
  - 각 영역별 에러 
  - 디버깅 편의 확인 
- 대규모 파싱 
  - 레전드 파일 이동 
  - 게임 프로토콜 전체 옮기기
  - 부족한 부분들 개선 작업 




완전한 테스트가 되려면 뭔가 서비스를 만들어야 한다.  그렇게 하려면 앞으로 나아가야 한다.  진행하면서 다시 보자. 



## 팩토리 등록 코드 

```c++
#include "file";

#define WISE_ZEN_ADD(cls) \ 
  wise::zen_factory::add( cls::get_topic(), [](){ return std::make_shared<cls>(); })

void add_file_messages() 
{
    WISE_ZEN_ADD(shop::req_buy_item); 
    // ...
}
```

바로 쓸 수 있도록 하는 게 좋다.  두 번 작업할 필요가 없다. 자동화 시킬수록 좋다. 

편하게 사용하는 방법은 다음과 같다. 

- 관리 단위가 너무 많지 않도록 메세지를 등록한다. 
- zen 파일 중심으로만 수정한다. 

visual c++에서 문법을 쉽게 볼 수 있도록 하는 게 좋다. C++과 거의 호환되니 C++ 파일로 등록해도 된다. 



## 에러 로그 상세하게 추가 

중간에 실패할 경우 원인을 알 수 있도록 로그 메세지를 추가한다. 상세해야 한다. 왜 실패 했는 지를 알 수 있어야 한다. 

현재 진행 단계를 파일과 노드 중심으로 보여준다. 실패하면 어디서 실패했는 지 알 수 있다. 




## 토픽 처리 

- 토픽
- 생성자 / 소멸자 
- 복사 / 할당 / 이동 



topic play.shop.req_buy_item;

T : string -> value

find T. 

- 사용하기 쉽고 
- enum처럼 ? 

그룹으로 처리되면 좋다. 타잎과 연결은 잘 모르겠다. 필드나 struct와 연결 될 수는 있다. 

전체 파일에서 찾아서 토픽을 등록. 다른 경우는 생성하지 않음.  

토픽 파일명 지정.  ml_topic.hpp 

- namespace
- category 
- 각 category enum 


GEN 엔진의 경우 thrift로 enum을 등록하고 사용한다. 이와 같이 사용자가 지정하는 방식으로 정리하는 것도 괜찮아 보인다. 

```c++
namespace topic 
    
enum category 
{
    
}

enum group 
{
    shop, 
}

enum shop 
{
    req_buy_item, 
}

```

사용할 때, `topic = play.shop.req_buy_item; ` 이라면 `(category::play, group::shop, shop::req_buy_item)`으로 코드를 생성한다. 

```c++
static const wise::topic& get_topic() 
{
    static wise::topic topic_( 
        topic::category::play, 
        topic::group::shop, 
        topic::shop::req_buy_item
    );
    
    return topic_;
}
```

namespace를 지정할 수 있어야 한다. 이는 프로젝트 전체에 적용되도록 한다.  옵션으로 처리한다. 

OK 



### 상속 지원 

꼭 필요하다.  LF는 많이 쓰고 있고, 레전드는 거의 안 쓰고 있다. 그래도 있으면 쓸 곳이 있다. pack / unpack 함수 호출만 연결하면 된다.  super 타잎을 using으로 지정한다. 생성자 / 소멸자 정의가 필요하다. 

base클래스를 두고 super::pack, super::unpack을 호출하도록 한다. 규약이 C++ / C#에 복잡할 수 있으므로 먼저 정리한 후에 다시 방문한다. 



## include 확인 

이전에 작업한 내용이 잘 동작한다. 

namespace는 :: 로 정의하고 사용한다. 



## pack / unpack

- 메세지 
- struct 

메세지는 내부에 함수를 갖고, struct는 wise namespace 안에 생성한다.  생성까지 잘 된다. 



## 초기화 표현식 

생각보다 복잡하여 별도 항목으로 해결한다.  일단, 정수형만 지원하도록 하고 나중에 확장한다. 변수를 포함하는 건 타잎 관련 문제들이 있는데 있으면 편하긴 하다.  타잎 체크를 포함한 처리에 대해 고민하고 정리한 다음 진행한다. 




## 코드 생성 

필드 위주로 타잎별 코드 생성을 한다.  기본 생성 흐름 확인 

- ustring 추가  OK
- 초기화 값들 
- 매크로 확인 OK



## 파싱 테스트 

코드 생성을 작성하기 전 기본적인 파싱이 잘 되는 지 확인한다. 파싱은 잘 된다. 한번 확인했던 내용이라 괜찮다. 



## idl_type 중심 처리 

field 중심에서 type 중심으로 구현을 변경한다. 커플링을 낮추고 확장성을 좋게 만드는 방법을 계속 생각한다. 

- simple 
- full 
- vec 
- map 
- topic
- macro 

생성할 때도 타잎별 코드 생성이 되도록 한다. 





## 간단한 파싱과 생성

```c++
enum types 
{
	v1, 
	v2, 
	v3,
}

// 아이템
struct item 
{
	u32 		id; 
	i32 		value;	
	string   	name;
	ustring  	desc;
}

msg req_buy_item 
{
	topic play.shop.req_buy_item;
	
	vec<item> items;	
	types 		tv;
}
```

위를 c++로 생성한다. 지정된 topic을 갖고 먼저 처리하도록 한다. 

생성된 코드는 다음과 같다. 

```c++
enum types 
{
	v1, 
	v2, 
	v3,
}

// 아이템
struct item 
{
	uint32_t 		id; 
	int32_t 		value;	
	std::string   	name;
	std::wstring  	desc;
}; 

// 제일 마지막에 inline 함수로 구현한다. 
namespace wise 
{
    template <> bool pack(zen_packer& packer, const item& v)
    {
        packer.pack(v.id); 
        packer.pack(v.value); 
        packer.pack(v.name); 
        packer.pack(v.desc); 
        
        return packer.is_valid();
    }
    
    template <> bool pack(zen_packer& packer, item& v)
    {
        packer.unpack(v.id); 
        packer.unpack(v.value); 
        packer.unpack(v.name); 
        packer.unpack(v.desc); 
        
        return packer.is_valid();
    }
} // wise


struct req_buy_item : public wise::zen_message
{
	vec<item> items;	
	types  	  tv;
	
	req_buy_item() 
	: zen_message( get_topic_cls() )
    {   
    }
    
    static const wise::topic& get_topic_cls() 
	{
		// 아래는 규약으로 맞추는 게 생성으로 처리하는 것 보다 나을 듯. 
		// C++과 C#의 enum 문법이 같다는 점을 활용한다. 		
		static topic tc(cat::play, cat_play::shop, grp_shop::req_buy_item);		
		return tc;
	}
	
	bool pack(wise::zen_packer& packer) override
    {
    	packer.pack(items);
    	packer.pack_enum(tv);
    	return packer.is_valid();
    }
    
    bool unpack(wise::zen_packer& packer) override 
    {
    	packer.unpack(items); 
    	packer.unpack_enum(tv);
    	return packer.is_valid();
    }   
}
```

위에서 cat::play, cat_play::shop, grp_shop::req_buy_item 등의 생성은 별도로 진행한다. 없으면 생성하고, 있으면 지나간다. 어디에 어떻게 둘 지는 좀 더 생각한다. 

추가로 생각할 내용은 많다. 

- factory 등록 코드의 생성과 사용 
- 메세지 핸들러 등록 

LF에서는 코드를 생성하고 include 포함해서 등록할 수 있게 했다. 코드 생성은 필요한데 확실히 좋은 방법인지는 아직 모르겠다. 



### 기본 타잎 추가 

u8 등 타잎 추가. 



### msg 키워드 추가 

message, msg 중 하나로 메세지 정의하도록 추가 



### vec 추가 

Field에 FieldType 추가. FieldType에 SimpleType / FullType / BasicType / ContainerType으로 나눠 처리. 파서와 심벌 테이블 구조를 변경해야 하므로 좀 큰 변경이다. 

배열 지원은 제거한다. vec / map 만으로 진행한다.  크기 제한을 둘 지 고려한다. 

타잎이 확장되어 idl_field 만으로 뭔가를 하기가 어려워졌다. 

idl_type의 하위 클래스를 확장한다.  idl_field는 더 단순하게 하고 타잎 쪽 기능을 올린다. 



dl/idl_parser.yy: conflicts: 2 shift/reduce, 17 reduce/reduce

- 단말 정보의 누락에 따른 충돌 

 field에 있던 기능을 type 중심으로 이동. 코드 생성을 위해 필요한 기능들 위주로 처리 



### topic 추가 

키워드로 추가하고 필드 중 하나로 한다. 










## 프로젝트 조정

wise 라이브러리 프로젝트를 추가한다.  테스트도 라이브러리 링크하도록 변경한다. 

boost를 포함하여 이전에 작업한 코드 빌드까지 진행했다. 



## 파싱 

기존에 작업된 내용에 기초해서 진행한다. 

- u8~u64 추가 
- vec  / map 타잎 추가 
- 배열 지원 제거 
- 토픽 
- 암호화 옵션 지정
- 초기값 설정 





## C++ 코드 생성 

- enum / struct / message
- factory 등록 코드 생성 
- topic 리턴 함수
  - req_move::topic() 
    - static 함수
- get_desc
  - type:topic




## C\# 코드 생성 

C# 클라이언트 전용으로 편하게 작업 가능하게 하는 것을 목표로 한다.  플래폼 호환을 위해 Async 통신 모델을 사용한다.  버그가 없어야 하고 안정적이어야 하며 충분히 빨라야 한다. 



# IDL 정의 

코드 생성과 별개로 IDL 스펙을 정의한다. 코드 생성과 Serialization은 각 프로젝트마다 요구 사항이 다르므로 별도로 처리하는 것이 맞다. 



# Example 

```c++
include "test/test2.zen"

namepsace test::mesage

enum hello 
{
  	v1, 
    v2, 
    v3 = 97
};

struct test 
{
    i32 v = 3; 
    float ratio = 3.0f; 
    double d = 3.0; 
    str  name;
    ustr friend_name;
}; 

cat play 
{
};

grp scene : play 
{
};

msg req_move : scene
{      
    uuid  who; 
    vec3  pos; 
    float orient;    
    vec<test> tests; 
    map<int, string> id_map;
}; 
```



## 타잎들

- i8~i64, u8 ~ u64 
- float, double 
- str, wstr
- vec, map 
- enum, struct, message


명령은 include만 있다. 타잎 선언 외에는 namespace 지정만 있다. 최대한 간결하게 한다. 



## translation 단위

- 단일 파일. include 파상하지 않음. 타잎 검증하지 않음 
- 단일 파일. include 파싱. 타잎 검증. 
  - 강제 진행 옵션이 없으면 파일 시간 비교하여 처리 

정리하면 해석 단위는 파일과 include 된 파일들이다. 파싱만 해서 심벌 테이블은 사용하고 코드 생성은 파일 시간 비교해서 결정한다. 이렇게 하려면 topic은 별도로 정의해야 한다. 



## 토픽과 상속 계층  

category, group을 별도 struct로 생성한다. 메세지와 동일하나 의미만 추가된다.  메세지의 토픽은 이들 상속 정보에서 자동으로 생성된다. 

### example

```c++
struct cat_play 
{
   static topic::category_t get_category() const 
   {
       return category::play;
   }
    
   enum 
   {
       scene, 
       ...
   }
};

struct grp_scene : public cat_play 
{
  static topic::group_t get_group() const 
  {
      return cat_play::scene; 
  }
  
  enum 
  {
      req_move, 
      res_move, 
      ...
  }
};

struct req_move : public zen_message, public grp_scene 
{  
  static topic pic() 
  {
     return topic(get_category(), get_group(), grp_scene::req_move);
  }
  
  req_move() 
     : zen_message( pic() )
  {   
  }  
};
```



TODO: zen_packer의 가장 일반적인 packing을 외부 템플릿으로 찾아서 처리 가능하게 한다. 그렇게 하면 ipackable 인터페이스를 사용하는 구현이 필요 없다. 



사용할 경우 다음과 같이 한다.

```c++
session::sub( topic(cat_play::get_category(), grp_scene::get_group(), 0), on_scene_message );
```

on_scene_message에서 적절한 scene을 찾아서 메세지를 넣어준다. scene_service에서 받아서 대상 scene으로 보내면 된다. scene은 task로 처리하고 어느 쓰레드에서 실행 될 지는 모르지만 큐에만 넣어주면 된다. 어느 scene일 지 세션 정보로만 찾을 수 있어야 하는데, 세션에 context를 지정하고 context에 있는 정보를 사용한다. 키 성격을 갖는 정보만 알면 되므로 atomic을 사용하여 쓰레드에 안전하게 지정할 수 있다. 

```c++
void on_scene_msg( message::ptr m )
{
    auto zp = std::static_pointer_cast<zen_message>(m);
    
    auto sref = network::inst().acquire(zp->sid);
    WISE_RETURN_IF( !sref );
    
    auto ctx = sref.get_session()->get_context(); 
    dispatch_to_scene( ctx->current_scene );    
}
```

위는 큐잉 지연을 갖거나, 락을 사용하지 않으면 안 된다. 이 보다는 채널을 안전하게 지정하는 방식이 나은 것으로 보인다. 

```c++
void on_scene_msg( message::ptr m )
{
    // ... 
    auto ctx = std::static_pointer_cast<per_game_context>(sref.get_session()->get_context()); 
    auto chn = ctx->get_scene_channel();
    
    if ( !chn )
    {
        // error. disconnect.  
        return;
    }
    
    chn->publish( m );        
}
```

get_scene_channel()은 락이 필요하다.  shared_timed_mutex를 사용하면 충돌은 최소화 할 수 있다. 

서비스 단위는 정적으로 처리되므로 괜찮다. 게임에서 scene이 유일하게 동적인 처리를 필요로 하는 지점이고 위와 같이 하면 안정적으로 처리 가능하다. 

세션을 상속하는 구조를 사용하지 않는 이유는 쓰레드 문맥이 명확하지 않기 때문이다. 구조와 실행이 최대한 일치해야 실수를 줄일 수 있다. 



## zen_packer 

클래스 내부에 템플릿을 갖고 serialize 하면 추가되는 타잎에 대한 코드 생성이 어려워진다. ipackable을 매번 상속 받으면 번거러워진다. 따라서, 외부에서 템플릿 함수의 특수화로 처리하면 더 나을 수 있다. 

```c++
namespace
{
struct vec
{
	float x, y, z;
};
}

namespace wise
{

template <> bool pack(zen_packer* packer, const vec& v)
{
	packer->pack(v.x);
	packer->pack(v.y);
	packer->pack(v.z);

	return packer->is_valid();
}

template <> bool unpack(zen_packer* packer, vec& v)
{
	packer->unpack(v.x);
	packer->unpack(v.y);
	packer->unpack(v.z);

	return packer->is_valid();
}

}
```

위와 같이 확장하는 방법이 깔끔하다. ipackable로 둘러싸는 건 여전히 유효하다.  코드 생성을 어느 쪽을 택할 지 선택해야 한다. inline 가능한 방법을 선택하는 것이 더 나을 듯 하다. 































