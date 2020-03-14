

예전에 작업하던 내용을 로그로 남긴다. 



# 개선 작업 

verbatim 블럭 처리 

```c++
cplus {
    
}
```

위와 같이 처리되도록 변경한다.  

이렇게 될 경우  {} 사이는 verbatim 블럭이 되는데 lexer에서 해석 없이 }가 나올 때까지 묶어서 하나의 tok_verbatim_block으로 돌려줘야 한다. 

이렇게 파싱 상태에 따라 lexer의 행동을 바꿔야 하는 경우들이 있다. bison 매뉴얼 "Handling Context Dpendencies"에 이런 문제에 대한 설명이 나와 있다. 

위의 처리가 가능한가? 

## 1안 - 렉서에서 처리 

"cplus"로 토큰이 매칭되면 파서가 아닌 렉서에서 처리를 해버리는 방법이 있다. '{' 에서 '}'까지 읽어서 파싱을 해버린다. 

문법 구조가 없는 것으로 처리해야 하기 때문에 그렇다고 할 수 있다. 

비슷하지만 '{'가 매칭 될 때 cplus가 설정되어 있으면 '}'까지 읽어서 넣는 방법이 있다. 아니면 '{'를 돌려준다. 



## 2안 - 파서에서 처리 

cplus를 토큰으로 인식하고 '{'를 만나면 '}' 까지 yyinput() 함수를 호출하면서 진행하는 방법이 있다. 



## 구현 

기본적으로 렉서 작업이다. 따라서, 1안으로 간다. verbatim_begin에서 하던 일을 "cplus"에서 한다. 원래 이렇게 하면 되는 거였다.  tok_cplus로 리턴해서 매칭 시켜주고 Capture에서 값을 지정하게 하면 된다. 





# 방향 다시 생각 

flatbuffers는 사용성이 문제다. 버퍼를 갖고 다녀야 한다.  성능도 문제가 될 수 있다. 
매우 효율적인 메모리 관리 기능이 없다면 오히려 문제가 될 수 있다.  가장 큰 문제는 사용이 매우 복잡하다는데 있다. 
버퍼 내 오프셋으로 계속 자료들을 찾아서 읽는 방식이라 여러 곳에서 메세지가 사용되면 
오히려 serialization 부담이 증가한다. 딱 맞는 곳이 있긴 할텐데 코드에 사용되는 메세지로는 적합하지 않은 듯 하다. 

- thrift에 해당하는 편리하고 빠른 자체 포맷을 만든다. 
- c++도 코드 생성을 통해 처리한다.  

Serializer 테스트를 하고 괜찮다는 생각이 든다. 기존 서버들을 붙이는 걸 쉽게 할 수 있으니 괜찮다. 

서버는 Little Endian으로 메모리 복사 형태로 serialize 하고 다른 언어나 플래폼에서 모두 리틀엔디안으로 맞춰서 통신한다.  
서버 부담은 적고 빠르게 처리 가능하다. 

thrift도 빅 엔디안에 맞추고 BitConverter.DoubleToInt64() 와 같은 함수를 사용하고 있다. 
현재는 이 정도가 적당해 보인다. 



## 정리 

기존의 방향은 괜찮다. r2c에서 m2c (메모리 복사 모델) 로 변경하고 generator를 각 프로젝트에 맞출 수 있도록 한다.  




# C\# pack/unpack 테스트 

- 기본 타잎 
  - 엔디안 
- 배열 
- 문자열
- enum  
- struct 

테스트 후 코드를 정리한 후 코드 생성에 반영한다.  C++ 과 호환 테스트를 진행한다.  

문자열 제외하고 배열과 기본 타잎들은 테스트 되었다. 특별히 문제 없어 보인다. 

## Utility 

통신 코드를 작성하려면 추가 고려할 사항들이 많다. 

- buffer와 통신 사용 
  - 메모리 관리 
  - 클라만 사용할 거라 큰 부담은 없다. 
  - 테스트 구현은 MemoryStream을 써도 된다. 
- Packer.Write(stream, int) 와 같은 함수들 
  - static constructor에서 endian 체크 
  - Endian을 처리하는 함수 
  - 문자열 배열 처리 등 
- Packer.Read(stream, out int)



## 기본 타잎 

Stream에 쓴다. Endian 처리를 한다.  

- Endian 
  - 서버를 little endian으로 가정 
  - 클라 엔디안 확인 
  - 컨버전 진행 
- Float / Double 
  - C++의 메모리 구조에 맞춰서 변환이 가능한가? 
  - 잘 될 수 있는가? 
  - IEEE-754 표준 
    - 아마도 될 것 같다
  - std::numeric_limits<float>::is_iec559 ? 
    - single precision IEEE standard 

약간의 위험이 있다. 어떤 포맷을 사용하는 지는 중요한 문제다. 네크워크 코드에 많은 영향을 미치고 플래폼 호환성에도 영향을 준다. 기반이라 더욱 중요하다. 

**진행:** 

- c++에서 Write --> c#에서 읽기 
- c#에서 Write --> c++에서 읽기 
- 기본 타잎들 모두 포함해서 처리 
- little endian (동일 장비)
- big endian으로 변환 후 다시 리틀 엔디언 변환 테스트 




### 문자열 처리 

string과 char / wchar_t 배열 간의 연동.  

- char와 string 간의 처리 
  - 인코딩 정보를 필요로 한다. 
  - 문자열과 바이트 배열의 구분이 안 된다. 
- wchar_t와 string 간의 처리 
  - 윈도우의 인코딩을 전제로 처리해야 한다. 

thrift는 어떻게 하고 있는가?

- std::string, string 
- UTF8으로 처리 
  - Encoding.UTF8.GetBytes(string)으로 C#에서 처리 
  - (uint8_t*)str.data()로 C++에서 처리 
  - UTF8인 것을 가정하고 있다. 

UTF8을 사용하는 것이 바람직해 보이긴 한다. 윈도우에서 잘 지원이 안 되는 게 문제이다. 

string / wstring의 인코딩을 셋업하게 하는 것은 어떨까? 배열 복사로는 잘 안 될 것으로 보인다.  

#### char 배열 

EUC-KR로 인코딩을 주고 처리한다.  각 언어별로 설정하도록 해서 처리해야 한다. 

일단, 잘 된다. 엔디안은 확인을 해야 한다. 

SL8, SL16 같은 클래스를 만들어서 생성해야 한다. 길이를 포함하고 있어야 한다. 

#### wchar_t 배열 

C#의 Unicode Encoding이 윈도우의 UTF16과 같다는 얘기가 있다. 그렇다면 이는 더 간단하다. 

이 쪽도 될 것으로 보인다. 항상 그런지는 모르겠다. 유니티의 각 타겟 플래폼에서 확인해 봐야 한다.  

#### 정리 

- ls8, ls16 타잎 추가. 
- 각 타잎에 따라 코드를 다르게 생성. 

### 결과 

float, double, 기본 타잎들 변환은 쉽게 잘 된다. Endian 처리는 특별히 필요 없다.  왜냐하면 비트 연산을 사용해서   리틀 엔디안으로 맞췄기 때문에 플래폼에 의존하지 않는다. 




## Factory 

메세지의 코드를 추가하고 지정된 enum 값에 따라 enum 코드 파일과 팩토리 등록, 생성을 하는 코드를 만든다. 이를 보고 찾아서 메세지를 만들고 Pack / Unpack을 할 수 있도록 한다. 



##  다국어 처리 

utf8 또는 WCHAR 처리. WCHAR가 현재 방식에는 좀 더 적합하다. char, wchar_t 타잎을 확인하여 처리에 포함. 



# 개선 작업



## 출력 폴더 

- 폴더 없으면 생성 
  - include 된 파일들의 폴더 생성 
  - 구조를 그대로 맞춤 



## 초기화 값 처리 

InnerExpression에 double, 문자열, bool 상수가 올 수 있도록 한다. 
이 값은 변수의 타잎에 따라 지정 가능한 값이 있으므로 구문 분석에서 체크해야 한다. 
문법 구조에서 타잎을 반영하기는 어렵다. 

이 부분도 시간 문제이다.  따라서, 방향을 확정할 때까지 대기한다. 


## 버전 관리 

메세지는 버전이라기 보다는 필드 집합을 맞추는 작업이다.  통신에 참여하는 모든 프로그램이 같은 
필드들을 사용한다고 하면 문제는 없다. 현실에서는 그런 차이가 많이 발생한다.  

세가지 정도 방법이 있다. 

- 전형적인 관리 방식은 피처별로 모두 같이 릴리스 되게 하는 방법이 하나 있다. 
- define을 사용하여 빌드 시 맞추는 방법이 있다. 
- 계속 추가만 하고 삭제를 하지 않는 방법이 있는데 이는 필드별로 시리얼라이즈를 하는 경우만 동작한다. 

두 번째 방법을 갖추는 게 필요하다. 

\#이 들어 있는 라인을 필드로 처리하고 생성 시 그대로 그 라인을 출력해주는 방법이 있다. 

```c++
{macro_line} {
  yylval.vline = _strdup(yytext);
  return tok_macro_line;
}
```

위와 같이 주고 field 중 하나로 처리하게 하면 된다. 

필드 serialize 코드에 반영해야 한다.  현재 macro_line을 기초로 verbatim_block을 구성해야 한다. 
verbatim block 내에 코드를 생성한다. 가능할 것으로 보인다. 

위와 같이 macro_line은 macro_line으로 이름을 변경한다. 

다음과 같이 동작할 수 있다: 
 - \#if면 if macro블럭으로 등록 
 - \#else면 else macro 블럭으로 등록 
 - \#endif면 현재 블럭의 종료 

각 필드에 대해 현재 블럭이 무엇인 지 지정한다. 이를 기반으로 serialization 코드 생성시 반영하여 처리한다. 


## include 처리 

include 전체를 순서대로 모음. 파일 시간을 보면 중복 처리가 되지는 않을 것이다. 
따라서, 그냥 처리를 진행해 나가면 된다. 먼저 처리해서 심벌에 넣어야 하므로 심벌만 보관한 별도 파일이 있어야 한다.
<이름, 타잎, 속성>을 갖는 json 파일로 만들고 이를 심벌 테이블로 로딩한다.

먼저 심벌 테이블 json 파일 생성 기능 없이 진행한다. 매번 생성하는 게
비합리적이긴 하나 json 파일 만들고 로딩하는 것도 일이다.
성능이 문제가 되면 매번 생성하는 걸 피하기 위해 심벌 파일을 작업한다. 

- \#include 출력 추가 



## Verbatim 블럭 

 thrift의 doc 처리. /**에서 */까지읽어서 문서로 처리하므로 이를 참고한다. 

어디에서 처리할 것인가?  Field 앞에서 lexer에서 처리하는 건 별로다. 파서에서 처리하는 게 좋다. 

cplus \$  VerbatimBlock \$   

위의 형태로 꼭 한다. 그래야 완전하다. 

$를 사용하면 멀티라인 주석처럼 처리가 가능하다. CaptureVerbatimBlock으로 처리할 수 있다. 

문법으로 처리하려면 }를 제외한 매칭을 가져와야 한다. 생각보다 자유롭지 않다. 일단 $로 진행하고 계속 생각해 본다. 

- doc text는 이미 키워드 이전에 설정된 것으로 가정한다. 
- $ $ cplus 로 해야 한다. 
- 일단, 추가되었다. 이 정도로 하고 다음으로 진행 



## 멀티라인 주석 

/*를 만나면 */까지 지나감.   thrift에서 옮겨서 추가. 



## 클래스 / enum 문서화

/**  */까지 각 definition의 주석 처리. 




# C# serialize / deserialize 

- Pack(MemroyStream ) 
- Unpack(MemoryStream)
- 최대한 단순한 방법을 사용 



# expression parsing 

제대로 안 됨. 생각 보다 복잡한 작업이다. calculator 작업 참고해서 진행 

- 배열의 크기에 있는 수식 처리가 제대로 안 됨 


- 변수 초기화 값 제대로 적용 안 됨 
  - `i8 sv = 33` 이 `byte sv`로만 적용됨
  - 띄어 쓰면 잘 된다. 붙여 써도 되게 만들어야 한다. 
- 상수들에서 +/- 사인을 제거하면 된다. 
  - 연산자로 가져오게 하려면 어떻게 해야 할까? 
  - 되었다. 토큰으로 '+', '-'를 만들고 파서에서 문법을 추가한다. 
    - sign 필드를 추가하는 것 



## 생성된 코드 설계

```c#

namespace r2
{

public enum Hello
{
	v1,
	v2,
	v3 = 97,
}

public partial class Test
{
	public int v1 = 33;
	public double ff;
	public Hello hv;
	public short[] v2 = new short[8];
    
    bool Pack(NetStream ms)
    {
       ms.Write(v1); 
       ms.Write(ff);
       ms.Write((int)hv);
       for ( int i=0; i<v2.Length; ++i)
       {
         ms.Write(v2);
       }
       return true;
    }
    
    bool Unpack(NetStream ms)
    {
       ms.Read(v1); 
       ms.Read(ff);
       ms.Read((int)hv);
       for ( int i=0; i<v2.Length; ++i)
       {
         ms.Read(v2);
       }
       return true;
	}
}

} // r2
```

NetStream 클래스를 MemoryStream을 사용해서 하나 만든다. 여기에 엔디언 처리를 포함한다.  타잎에 따라 생성만 하고 Pack(), Unpack()을 호출하면 된다. IPackable에서 상속 받도록 한다. 

생성된 코드를 가정하고 테스트 한 후 거기에 맞춰 코드를 생성한다. 그러면 테스트가 이미 된 것이다.  C++ 코드 생성도 한다.  오늘은 C++ 코드 생성까지 진행한다.



## C++ 코드 생성 

함수 생성은 아직 진행하지 않았다.  namespace를 통한 참조에 대해 정리가 필요하다.  

- InnerExpression에서 문제가 발생 
  - FullType '.' tok_identifier 가 제대로 동작하지 않음 

c++ 과 c# 모두 namespace와 클래스 변수 접근이 같은 문법을 사용한다. 문맥에 따라 다르다. 이 과정이 무한히 나올 수 있으므로 개념 정의와 구현이 필요하다.  



다음을 정리하고 Pack / Unpack 코드로 넘어간다. 

- namespace, class 참조의 여러 단계 지원 
  - 재귀로 처리. 간단하게 잘 된다. 일반적으로 처리하는 게 더 쉬울 때가 많다. 
- C++의 Get / Set 함수들 생성 
- C++의 생성자와 C++ 블럭 지원 
- \# 라인들 그대로 출력 



이번 주 다 투입하면 마무리 될 것으로 보인다.  



 ## Thrift 코드 생성 참조 

역시 생성된 코드라 길고 보기에 좋지는 않다. partial 클래스로 추가하여 다른 곳에서 애플리케이션에서 필요하게 수정 가능하다. 

```c#

  #if !SILVERLIGHT
  [Serializable]
  #endif
  public partial class Sampel1 : TBase
  {
    private int _field1;
    private int _field2;

    public int Field1
    {
      get
      {
        return _field1;
      }
      set
      {
        __isset.field1 = true;
        this._field1 = value;
      }
    }

    public int Field2
    {
      get
      {
        return _field2;
      }
      set
      {
        __isset.field2 = true;
        this._field2 = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool field1;
      public bool field2;
    }

    public Sampel1() {
    }

    public void Read (TProtocol iprot)
    {
      iprot.IncrementRecursionDepth();
      try
      {
        TField field;
        iprot.ReadStructBegin();
        while (true)
        {
          field = iprot.ReadFieldBegin();
          if (field.Type == TType.Stop) { 
            break;
          }
          switch (field.ID)
          {
            case 1:
              if (field.Type == TType.I32) {
                Field1 = iprot.ReadI32();
              } else { 
                TProtocolUtil.Skip(iprot, field.Type);
              }
              break;
            case 2:
              if (field.Type == TType.I32) {
                Field2 = iprot.ReadI32();
              } else { 
                TProtocolUtil.Skip(iprot, field.Type);
              }
              break;
            default: 
              TProtocolUtil.Skip(iprot, field.Type);
              break;
          }
          iprot.ReadFieldEnd();
        }
        iprot.ReadStructEnd();
      }
      finally
      {
        iprot.DecrementRecursionDepth();
      }
    }

    public void Write(TProtocol oprot) {
      oprot.IncrementRecursionDepth();
      try
      {
        TStruct struc = new TStruct("Sampel1");
        oprot.WriteStructBegin(struc);
        TField field = new TField();
        if (__isset.field1) {
          field.Name = "field1";
          field.Type = TType.I32;
          field.ID = 1;
          oprot.WriteFieldBegin(field);
          oprot.WriteI32(Field1);
          oprot.WriteFieldEnd();
        }
        if (__isset.field2) {
          field.Name = "field2";
          field.Type = TType.I32;
          field.ID = 2;
          oprot.WriteFieldBegin(field);
          oprot.WriteI32(Field2);
          oprot.WriteFieldEnd();
        }
        oprot.WriteFieldStop();
        oprot.WriteStructEnd();
      }
      finally
      {
        oprot.DecrementRecursionDepth();
      }
    }

    public override string ToString() {
      StringBuilder __sb = new StringBuilder("Sampel1(");
      bool __first = true;
      if (__isset.field1) {
        if(!__first) { __sb.Append(", "); }
        __first = false;
        __sb.Append("Field1: ");
        __sb.Append(Field1);
      }
      if (__isset.field2) {
        if(!__first) { __sb.Append(", "); }
        __first = false;
        __sb.Append("Field2: ");
        __sb.Append(Field2);
      }
      __sb.Append(")");
      return __sb.ToString();
    }

  }
```



# C\# 코드 생성 개선 작업 

```c#
using System;

namespace r2
{

public enum Hello
{
	v1,
	v2,
	v3 = 97,
}


public class Test
{
i32 v1;
double ff;
i8 v2[3];
na.h c;
na.h c[Hello.v3];
}


public class Level2
{
Test a;
Test a[2];
}

} // r2

```



- 필드를 public으로 만들기. property? 

- 타잎들 맞추기 

  - 기본 타잎들 변환 

    - 각 bool, short, int, long, float, double
    - C#에서 int는 Int32, long은 Int64가 됨 

  - 배열 

  - 클래스와 enum 구분 

    - 심벌 테이블에서 찾아서 구분해야 함


# C\# 코드 생성 

 - 골격 생성 
   - node를 따라서 진행 
   - 생성하면서 개선해 나감 
   - namespace가 없으면 c# 코드를 생성할 수 없음 
     - json으로 기본 구성에 대한 설정이 필요 
	 - 일단은 진행하고 정리해서 json으로 옮김 

 - 프로그램 문법의 구조
   - 시작과 종료 (블록)
   - 순서 
   - 재귀 (include) 

 - 세부 serialization 코드 생성 
   - simple 
   - full 

 - Expression 파싱이 안 됨 
   - operator로 인식하지 못 함 
   - 한다고 해도 문제가 될 수도 있음 
   - 일단, 기본 골격 생성에 집중하고 처리 
   - 여러 단계 namespace 지정과 함께 처리 
   

# Code cleanup 

 - check pointers 
 - delete pointers (destructor)

# Debugging AST 

 - g_program으로 상태를 파악한다. 

 - idl_parser.yy에 브레이크를 걸어야 동작한다. 
   - \#line 때문에 그렇다.  

# C\# 코드 생성


C# 코드 생성을 목표로 AST를 구성하고 generator 작업을 진행한다.  

idl_program이 무엇을 갖고 있어야 하는가? 

- TypeDefinition들 
  - enum, struct, message 
  - 생성을 위해 필요 
  - 검증을 위해서는 전체 include된 파일들의 심벌이 필요 


- include 처리 
  - idl_program별로 parsing 진행 
  - symbol table은 함께 사용 
    - namespace 기반의 검증 

```c++
enum FieldSize 
{
	embed_size = 10, 
    pos_size = 3
};

struct Position
{
	float x = 0; 
	float y = 0; 
	float z = 0;
};

struct Embed
{
	i8 f1;
	Position pos[FieldSize.pos_size];
};

struct Test01
{
	i8 f1; 
	i16 f2; 
	i32 f3; 
	i64 f4; 
	
	float f5; 
	double f6; 
	
	i8 a1[3];	
	float a2[3];

	Embed e[FieldSize.embed_size];
};
```

대부분의 기능을 갖는 샘플이다. 위는 거의 유효한 c++ 코드이며 c#의 pack/unpack 코드만 잘 만들어지면 된다. 위의 내용으로 테스트를 진행한다. 

## Semantic Actions 

- enum 
- struct 
- field 
- simple / full type
- array 

###방향 

Node 기반으로 트리를 구성하고 진행할 지 아니면 thrift 처럼 그냥 program에 타잎별로 추가해서 진행할 지 생각을 좀 해야 한다. 
Node기반으로 하면 다른 파서 / 생성기 작성할 때 유사한 패턴을 유지할 수 있다.  
thrift도 동일한 개념을 트리로 구성하지 않고 개념에 맞게 직접 구성했다는 점만 다르다. 

따라서, Node와 자식으로 구조를 만드는 게 좋을 듯 하다. 

bison의 c++ 예제를 작성한 사람이 계산기의 AST를 트리로 구성하는 예를 작성했으므로 이에 기초하여 진행한다. 

- idl_node
  - print() = 0;
  - indent
  - children
- idl_node_enum 
  - enum_value 
- idl_node_struct 
  - field 

symbol_table을 갖는다. namespace를 포함한 full_name이 키가 된다. 현재 파일의 namespace를 붙인다.  
진행을 실제 적다 보니 크게 의미가 없어졌다. 각 언어에 영향을 받는 부분이 많다. 
오히려 Syntax Directed Translation을 잘 이해하는 게 더 중요하다.

## enum 

- new 하는 지점 
- append로 값들 추가 
- program 추가 시점 

thrift 의 동작을 따르면 된다. SimpleExpression의 처리는 많이 다르다.  




## struct 

여길 다하면 거의 다 하는 것이니 이 쪽을 먼저 진행한다. 

- 최상위에서 시작한다. 
- FieldList와 같은 형태는 empty 규칙이 가장 먼저 실행된다 

```c++
SimpleExpression: 
   SimpleExpression '+' SimpleExpression
	{
		util::log()->debug("SimpleExpression => SimpleExpression '+' SimpleExpression");

		// shift / reduce conflict가 있지만 괜찮다. 

	}
| SimpleExpression '-' SimpleExpression 
	{
		util::log()->debug("SimpleExpression => SimpleExpression '-' SimpleExpression");
	}
| FullType '.'  tok_identifier 
	{
		util::log()->debug("SimpleExpression => FullType '.'  tok_identifier");
	}
| tok_identifier 
	{
		util::log()->debug("SimpleExpression => tok_identifier");
	}
| tok_int_constant 
	{
		util::log()->debug("SimpleExpression => tok_int_constant");
	}


```

좀 까다로운 형태이다. 복합문에서는 add_expression()만 한다. FullType 등에서 expression을 만든다. 

### Symbol Validation 

코드 생성할 때 뒤지겠지만 파싱할 때 미리 오류를 발견할 수 있다면 좋다. symbol_table에 등록을 해야 하는데 적절한 시점이 어디일지 파악해야 한다. 먼저 시맨틱 액션이 동작하도록 한 다음 코드를 추가하는 게 좋을 듯 하다. 

## AST 생성 

enum / struct에 대해 추가했다. 하지만 깔끔하지 않다. 그래서, 최상위로 program에서 보관해야 하는 것들은 node로 한다. 

- idl_node
  - idl_node_struct 
  - idl_node_enum 
  - idl_node_include 

타잎으로 처리되는 것만 type으로 한다. 

- idl_type 
  - idl_type_simple
  - idl_type_full

나머지는 struct, enum, message 등에 추가되므로 그냥 이름을 쓴다. 

- idl_enum_value 
- idl_field
- idl_field_value
- idl_expression 

expression만 처리하고 디버깅하면 이제 코드 생성으로 넘어갈 수 있다.  

이제 테스트를 진행하고 코드 생성을 하면서 더 살핀다. expression이 좋은 기능이긴 하다.  CSP를 공부하면 의미론에 대해 더 다가갈 수 있다. 



# 로거  

로거를 붙여서 처리한다.  에러만 붙임. 다른 것들은 semantic action들 중심으로 처리되도록 한다. 


# 주석문 처리 

{comment}를 lex 규칙에 추가하여 처리. c 주석은 일단 지원하지 않음. 



# 간단한 expression 처리

Hello.v1 + 3*2 

위와 같이 간단한 수식의 처리가 필요하다. enum의 tok_int_constant 대신 사용할 수 있어야 한다. enum 사용과 수식을 지원한다. 

SimpleExpression: 

   SimpleExpression '+' SimpleExpression

| SimpleExpression '-' SimpleExpression 

| FullType '.'  tok_identifier 

| tok_identifier 

| int_constant 

위의 정도로 충분해 보인다. AST 생성 시 좀 더 다듬도록 한다. 

```
State 62 conflicts: 2 shift/reduce
State 63 conflicts: 2 shift/reduce
```

```
   33 SimpleExpression: SimpleExpression '+' SimpleExpression
   34                 | SimpleExpression '-' SimpleExpression
   35                 | FullType '.' tok_identifier
   36                 | tok_identifier
   37                 | tok_int_constant
```

위의 정의에서 각각 shift / reduce conflict가 발생했다.  

```
State 62

   33 SimpleExpression: SimpleExpression . '+' SimpleExpression
   33                 | SimpleExpression '+' SimpleExpression .
   34                 | SimpleExpression . '-' SimpleExpression

    '+'  shift, and go to state 55
    '-'  shift, and go to state 56

    '+'       [reduce using rule 33 (SimpleExpression)]
    '-'       [reduce using rule 33 (SimpleExpression)]
    $default  reduce using rule 33 (SimpleExpression)
```

각 더하기, 빼기 기호에서 shift와 reduce가 동시에 발생한다. bison은 이를 처리하기 위해 shift를 먼저 하도록 되어 있고 (더 긴 길이를 파싱하게 된다) []로 쌓인 부분이 무시된 reduce를 표시한다. 마법이다. 



# 네임스페이스 처리 

SimpleType 외에 FullType 으로 namespace를 갖는 변수 선언이 가능하도록 한다. 

복잡하게 처리하기 보다는 한계를 두는 게 좋을 듯 하다. 

```
FullType: 
  tok_identifier tok_namespace_separator tok_identifier 
	{
	}
```

위의 정의로 한 단계 namespace만 지원한다. FnApp::Struct1과 같은 형태만 지원한다. 



# Flex / Bison 기반 파서 

단계적으로 진행한다. C 스타일의 렉서 / 파서를 사용한다. (thrift도 이에 기초)

## enum 

enum identifier { 
	age = 3, 
	value = other_enum + 100, --> 나중에. thrift도 const 값만 지정하게 되어 있음. 확장이 필요한 이유 중 하나
};

### thrift의 문법 

- %type<tenum>     Enum
- %type<tenum>     EnumDefList
- %type<tenumv>    EnumDef
- %type<tenumv>    EnumValue

## 구조의 정리 

idl_parser.yy에서 idl_parser.cpp와 idl_parser.hpp를 생성. 

idl_lexer.ll에서 idl_lexer.cpp를 생성 

idl_main.cpp의 parse() 함수에서 파싱을 시작한다. 


## Rule의 처리 

- 매칭 되었을 때 처리 행동의 구현
- 언제 new로 생성하는 지?
- $$, $1들은 어떻게 동작하는 지?

이 부분 해결해야 자연스럽게 처리가 가능하다. 

- AST 구성하는 사람 마음이다. 
- $$는 타잎을 정확하게 보면 된다. (group 또는 nonterminal의 타잎)
- $1부터 \$n까지도 그렇게 된다. 



## enum 

언어 정의 디버깅을 위해 --verbose나 --report를 추가하면 .output 파일이 생성된다. 코드 생성의 기준을 제시하므로 언어 정의 내의 버그를 찾기 쉽다. 

- enum 인식 못 하는 문제 
  - 앞에 identifier가 있어서 인식 못 한 것으로 보임 
  - 문자열 매칭은 제일 앞으로 올리는 것이 좋음 
- v1 (identifier) 오류 
  - identifier로 매칭 안 되는 것으로 보임 
- 파서 실행 디버깅 
  - YYDEBUG 를 1로 정의 
  - yydebug 값을 1로 지정 
  - 상태 전이, 토큰 타잎이 보임. 
  - 개발 시에는 항상 켜고 봐야 할 듯 

### enum identifier { identifer }에서 { 인식이 안 됨 

'{' 값이 파서에 정의되어 있는데 identifier 인식이 안 됨. 

```
Starting parse
Entering state 0
Reading a token: Next token is token tok_enum ()
Shifting token tok_enum ()
Entering state 1
Reading a token: Next token is token tok_identifier ()
Shifting token tok_identifier ()
Entering state 5
Reading a token: {Next token is token tok_identifier ()
[ERROR::3] (last token was 'v1')
syntax error
Error: popping token tok_identifier ()
Stack now 0 1
Error: popping token tok_enum ()
Stack now 0
Cleanup: discarding lookahead token tok_identifier ()
Stack now 0
```

state 5에서 state 7으로 넘어가는 게 보이지 않는다. enum과 enum의 identifier를 읽고 { 에서 상태 변경이 일어나지 않았다. 왜 그런가? 

 ```
State 5

    3 Enum: tok_enum tok_identifier . '{' EnumDefList '}'

    '{'  shift, and go to state 7
 ```

state 5는 '{'를 만나면 state 7으로 가게 되어 있다. 그런데 가지 않았다. %error-verbose를 yy파일에 추가. 에러 메시지는 `syntax error, unexpected tok_identifier, expecting '{'`로 나옴. '{'를 어딘가에서 사용한 것으로 보인다. 

어디서 누락되는 지 디버깅이 쉽지 않다. 

tok_open_brace, tok_close_brace로 추가하고 지나갔다. 이제는 = 문자에서 문제가 발생했다. 토큰으로 만드니까 잘 된다. 왜 문제가 생겼을까? 

중괄호로 디버깅을 해서 원인을 찾는다. 

**디버깅:** 

- 뭔가 옵션이 있는 것인가? 
  - 옵션 때문은 아닌 듯 하다. 
- 기본 룰 (lexer의 default 룰)로는 토큰으로 전달하지 않는다. 
- symbol을 룰로 만들어서 해결 (thrift에는 그렇게 이미 되어 있는데 누락해서 생긴 문제)

### 구조를 단단하게 만들고 진행하기 

- 디버깅 쉽도록 만들기 
  - %error-verbose랑 yydebug 매크로 기능으로 어느 정도 쉽게 됨 
  - lexer 의 토큰 출력하게 만들기 
    - 어디까지 어떻게 매칭되었는 지 알기 어렵다. 
    - 이를 디버깅 가능하게 한다. 

flex -d --wincompat -o "idl\\idl_lexer.cpp" idl/idl_lexer.ll && bison -y -d --verbose -o "idl\\idl_parser.cpp" idl/idl_parser.yy

- flex -d 옵션은 디버깅이 가능하게 한다. 
- --verbose 모드는 output 파일을 만들어 파서 동작을 볼 수 있게 해준다. 
- -y 옵션은 yacc 호환모드이다. 

flex -d와 --verbose, YYDEBUG 기능으로 충분히 잘 디버깅할 수 있게 되었다. 파싱 과정 전체를 상세하게 이해하면 더 쉬워질 것이다.  올해는 컴파일러와 시맨틱스의 해이다. 

## struct

기본 파싱은 쉽게 진행되었다. 추가 기능은 다음과 같다. 

- 일반 타잎의 처리 
  - namespace 처리 
- 배열 타잎의 처리 
  - expression의 사용. 
    - enum 과 정수 상수 값의 수식 
    - enum에서도 사용할 수 있는 expression 



## message 

여기는 현재로서는 의미가 없으니 struct로 테스트 하면 된다. 





## 파스 트리와 심벌 테이블 

 - Type 
   - EnumType 
     - EnumFieldType
   - StructType 
   - MessageType
   - FieldType 
     - PrimitiveType
	 - ArrayType
   - ConstType

## 문제해결 

 - expression 검증
   - assignment 
     - enum value 
	 - default value
   - array



## 자료 

 - thrift 코드를 주로 참조 
 - https://www.lysator.liu.se/c/ANSI-C-grammar-y.html
   - C 문법 정의 참조 



# 단계적인 파서 / 생성기 구현 

 - C# 코드 생성을 먼저 진행 
 - Pack(Stream), Unpack(Stream) 인터페이스를 구현 
 - 같은 엔디언으로 우선 진행 
 - 인코딩은 마지막 처리 


## vs2015 이슈 

spirit x3를 사용할 때 decltype을 통한 추론에서 문제가 발생한다. vs2017에서는 제대로 동작한다. 
이처럼 c++의 template과 타잎 관련 기능 발전은 상황을 성가시게 만든다. 
차라리 수작업으로 파싱을 하는게 나을 수도 있는데 연습이 충분하지 않다. 
flex와 bison을 사용해서 안정적으로 만드는 게 나을 수도 있다. 

qi는 x3의 장점이 없고 사라질 코드라는 게 문제다. x3는 rule과 attribute 기반으로 ast를 잘 만들어 준다. 
이 쪽으로 진행해도 되겠지만 여러 가지 문제가 나올 수 있다. 따라서, flex, bison을 다시 시도한다. 


## 작업 순서 

 - enum 
 - struct
   - variable_decl  
 - message   
 - namespace 
 - include 


## c# 코드 생성 

### Primitive Types 

 - Pack() 
   - Serializer.Pack( stream, member ); 
   - ... 
 - Unpack()
   - Serializer.Unpack( stream, member ); 

### enum 

 - int32 or int64? 
 - check on x86 and x64 build 

### struct 

   member.Pack(), member.Unpack()

### array 

   serialize each one with the size 

   Code generation: 
   - array 
   - 길이만큼 순화화면서 Pack/Unpack 호출 
   - 타잎의 struct / enum / primitive / 문자열 구분 

### 문자열 

 인코딩 해서 배열 복사. 



# project setup 

- 기본 유틸리티 추가 
  - 로그 (spdlog) 
  - 매크로 
  - exception 

- 경로 추가 
  - BOOST_HOME 필요 


# 명령줄 option 처리 

- boost.program_options 사용 
  - http://www.boost.org/doc/libs/1_58_0/doc/html/program_options/tutorial.html
  - 언어 옵션 
    - cplus / cpp 
	- csharp
  - 강제 옵션 
    - force
  - 문법 체크만 
    - parse 
	- 파스 트리 출력 
	- 에러 없으면 전체 진행 
  - 필요시 추가하면서 진행 
    

# 파싱 정보 출력과 재사용 

전체 툴의 안정성과 성능을 위해 필요. 
json 형식으로 남김. 

각 AST 항목을 파일로 남기고 재사용함으로써 전체 효율을 올림

# 파서

- enum 파서
- struct / message 파서
  - c# 코드 생성
  - (de)serialization 테스트
- c++ 코드 생성
  - (de)serialization 테스트
- 에러 핸들링은 진행하면서 중간 중간 추가

- include 처리
- namespace 처리
- using 처리

- 검증 기능 추가
  - 어느 정도 수준으로 할 지 결정
  - flatbuffers를 기준으로 함
    - enum 검증이 있다. 
	- r2c는 추가 검증이 더 필요하다. 
	

# 컨벤션 

spirit과 boost에 의존하고 R2 코드와 구분하기 위해 boost convention을 사용한다. 

- 모든 이름은 소문자와 언더바를 사용 
  - 매크로는 대문자 사용 
  - 템플릿 아규먼트는 PascalCase 사용 

- 주석 
  - 코드가 문서가 되도록 중요한 개념들에 대해 주석 달기 
  - 구현 중 어려운 부분에 대해 주석 달기 



# 설계 및 구현 

## 파서들 

작은 파서들부터 구현하고 코드 생성까지 진행. 테스트 후 다음 단계로 진행. 



# Reading Manual 

룰 처리 부분을 이해하기 위해 디버깅을 통해 코드 흐름을 파악하고 개념을 이해해야 한다.  bison 매뉴얼을 읽으면서 따라간다. 

- LALR(1), GLR parsing 
- token / grouping for terminal and non-terminal symbols 
  - token에서 그룹을 만드는 작업
  - bottom-up (LR) parser라서 이렇게 설명한다. 
- token type and semantic value 
- semantic actions 
  - executed on a match 
  - get semantic value from the semantic values of its parts
- GLR에 대한 설명 
  - 여러 개의 파서들을 만들고 실행해서 선택하는 개념 
  - 언어 정의에서 배제하는 게 좋음 
  - 필요할 경우가 있나 본데 나중에 볼 것 
- @$, @1, @3 
  - 로케이션 관련해선 특별히 할 일이 없다. 
  - semantic action의 경우 값을 만드는 게 필요 

## Bison Grammar Rules 

### Outline 



- Prologue 
- Bison declarations 
- Grammar rules 
- Epilogue 

#### Prolog 

여러 개 올 수 있다. YYSTYPE이 %union의 타잎을 의미한다. 

%code로 대체할 수 있다. top, requires, default (빈). %code top { }, %code requires {} 

requires는 헤더 파일로 가거나 위로 간다. 

### Symbols, Terminal and Nonterminal 

토큰들은 다음과 같이 정의된다. 

- named token type 
- character token type 
- literal string token 

-d 옵션으로 빌드하면 매크로 정의를 헤더에 추가하여 lexer에서 사용 가능하도록 한다. 

### Grammar Rules

- %empty for empty rule 
- use left recursion 
  - to use only bounded size of stack 
- mutual recursion 

### Defining Language Semantics

- %define to define data type of all constructs or %union for diffrent types 
- %token or %type to specify types for token or groupings 

Actions: 

- $n 
- \$\$ 
- yylval : lookahead value 
- Mid-Rule Actions 
  - 대부분의 경우 사용 안 하니 일단 스킵

### Tracking Locations 

- YYLTYPE 정의. 
- @$로 접근. @1, @3 등으로 접근 

### Named References

이름을 사용하거나 이름을 줄 수 있다. 

```c++
invocation: op ’(’ args ’)’
{ $invocation = new_invocation ($op, $args, @invocation); }
```

혼선이 생길 경우 exp[left]와 같이 줄 수 있다. 



### Bison Declarations 

- token type names
  - %token name 
  - or %left, %rifht, %precedence, %noassoc 
  - %token <Type> ID 
    - 타잎을 주기위해 사용 
- nonterminal symbols (grouping)
  - %type <Type> nonterminal 
- %destructor 
  - 메모리 해제에 사용 
  - 배치 파서에서는 크리티컬한 문제는 아님 
  - thrift는 에러 발생시 해제 하는 것으로 함 


# Syntax Directed Translation 

파스 트리의 각 노드에 속성 값을 주고 이를 기반으로 의미 해석을 진행한다. 

- attribute grammar 
  - attributed attached to syntax
  - each production in a grammar, give semantic rules or actions
- inherited attribute 
  - semantic rule associated with the production at the parent of N. 
  - attribute values at N's parent, N itself and N's siblings
  - 타잎 계산 같은 곳에 유용 
- synthesized attribute 
  - 각 노드에 주어진 하나의 속성 값 
  - attribute values at children of the node 
  - attribute value at node itself 
  - S-attributed definitions 
    - bottom up에 유용
- annotated parse tree
  - 값을 함께 갖고 있는 트리 
- L-attributed definitions
  - 왼쪽에서 오른쪽으로 루트에서 시작 
  - 상속과 합성 값 모두를 사용 

SDT는 SDD와 다르다. 위의 내용이 SDD에 해당. SDT는 문법 생성 규칙에 행위를 붙인 것. bison의 접근 방식이 이것이다.  
SDT는 SDD에 의존하고 L-attributed SDD를 사용한다 (preorder action traversal). bison은 synthesized attribute를 주로 사용한다. 
왜냐하면 \$1, \$n이 자식 노드이기 때문에 자식 노드의 값에만 접근할 수 있다. 


# TODO

- c# 코드 생성
  - 기본 타잎들
  - 문자배열
  - 배열
- c# 코드 테스트 
- c++ 코드 생성과 테스트 
  - c#가 매우 유사하다. 
- include
- message
- 버전 관리  
  - 어떻게 할 것인가? sequence로 추가만 가능하게? 
- 문법 개선 
  - double, float 초기화 값 지정 가능하게 함 
  - namespace 사용 시 여러 개 가능하게 함  OK
- 심벌 체크 
  - c#의 경우 전체 참조가 가능하여 점검이 불가능하다. 
  - c++의 경우는 가능하다. 나중에 구현한다. 
- 생성 코드 중복 
  - 비슷한 부분이 많은데 이를 어떻게 할 지 고민해야 한다. 
  - 중복 제거는 좋은 프로그래밍 연습이다. 
- verbatim 블럭 추가 OK
  - cplus { }, csharp {} 형태로 사용 
  - 주석의 처리 과정을 살펴보고 작업
- namespace OK
- AST 생성 OK
  - Seems  
  - Add cleanup code
  - Add check code 
- message node 추가 OK
  - 여기를 시작 점으로 한다
  - 일단, struct와 동일하게 생성
  - c++과 c# 모두 추가 코드 생성이 필요하다
- c++ 접근 함수 생성 
  - 필드들 private로 생성 
  - public 함수들 생성 
  - 타잎에 대해 적절한 처리를 포함 

