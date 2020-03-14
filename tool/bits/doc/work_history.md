

������ �۾��ϴ� ������ �α׷� �����. 



# ���� �۾� 

verbatim �� ó�� 

```c++
cplus {
    
}
```

���� ���� ó���ǵ��� �����Ѵ�.  

�̷��� �� ���  {} ���̴� verbatim ���� �Ǵµ� lexer���� �ؼ� ���� }�� ���� ������ ��� �ϳ��� tok_verbatim_block���� ������� �Ѵ�. 

�̷��� �Ľ� ���¿� ���� lexer�� �ൿ�� �ٲ�� �ϴ� ������ �ִ�. bison �Ŵ��� "Handling Context Dpendencies"�� �̷� ������ ���� ������ ���� �ִ�. 

���� ó���� �����Ѱ�? 

## 1�� - �������� ó�� 

"cplus"�� ��ū�� ��Ī�Ǹ� �ļ��� �ƴ� �������� ó���� �ع����� ����� �ִ�. '{' ���� '}'���� �о �Ľ��� �ع�����. 

���� ������ ���� ������ ó���ؾ� �ϱ� ������ �׷��ٰ� �� �� �ִ�. 

��������� '{'�� ��Ī �� �� cplus�� �����Ǿ� ������ '}'���� �о �ִ� ����� �ִ�. �ƴϸ� '{'�� �����ش�. 



## 2�� - �ļ����� ó�� 

cplus�� ��ū���� �ν��ϰ� '{'�� ������ '}' ���� yyinput() �Լ��� ȣ���ϸ鼭 �����ϴ� ����� �ִ�. 



## ���� 

�⺻������ ���� �۾��̴�. ����, 1������ ����. verbatim_begin���� �ϴ� ���� "cplus"���� �Ѵ�. ���� �̷��� �ϸ� �Ǵ� �ſ���.  tok_cplus�� �����ؼ� ��Ī �����ְ� Capture���� ���� �����ϰ� �ϸ� �ȴ�. 





# ���� �ٽ� ���� 

flatbuffers�� ��뼺�� ������. ���۸� ���� �ٳ�� �Ѵ�.  ���ɵ� ������ �� �� �ִ�. 
�ſ� ȿ������ �޸� ���� ����� ���ٸ� ������ ������ �� �� �ִ�.  ���� ū ������ ����� �ſ� �����ϴٴµ� �ִ�. 
���� �� ���������� ��� �ڷ���� ã�Ƽ� �д� ����̶� ���� ������ �޼����� ���Ǹ� 
������ serialization �δ��� �����Ѵ�. �� �´� ���� �ֱ� ���ٵ� �ڵ忡 ���Ǵ� �޼����δ� �������� ���� �� �ϴ�. 

- thrift�� �ش��ϴ� ���ϰ� ���� ��ü ������ �����. 
- c++�� �ڵ� ������ ���� ó���Ѵ�.  

Serializer �׽�Ʈ�� �ϰ� �����ٴ� ������ ���. ���� �������� ���̴� �� ���� �� �� ������ ������. 

������ Little Endian���� �޸� ���� ���·� serialize �ϰ� �ٸ� �� �÷������� ��� ��Ʋ��������� ���缭 ����Ѵ�.  
���� �δ��� ���� ������ ó�� �����ϴ�. 

thrift�� �� ����ȿ� ���߰� BitConverter.DoubleToInt64() �� ���� �Լ��� ����ϰ� �ִ�. 
����� �� ������ ������ ���δ�. 



## ���� 

������ ������ ������. r2c���� m2c (�޸� ���� ��) �� �����ϰ� generator�� �� ������Ʈ�� ���� �� �ֵ��� �Ѵ�.  




# C\# pack/unpack �׽�Ʈ 

- �⺻ Ÿ�� 
  - ����� 
- �迭 
- ���ڿ�
- enum  
- struct 

�׽�Ʈ �� �ڵ带 ������ �� �ڵ� ������ �ݿ��Ѵ�.  C++ �� ȣȯ �׽�Ʈ�� �����Ѵ�.  

���ڿ� �����ϰ� �迭�� �⺻ Ÿ�ٵ��� �׽�Ʈ �Ǿ���. Ư���� ���� ���� ���δ�. 

## Utility 

��� �ڵ带 �ۼ��Ϸ��� �߰� ����� ���׵��� ����. 

- buffer�� ��� ��� 
  - �޸� ���� 
  - Ŭ�� ����� �Ŷ� ū �δ��� ����. 
  - �׽�Ʈ ������ MemoryStream�� �ᵵ �ȴ�. 
- Packer.Write(stream, int) �� ���� �Լ��� 
  - static constructor���� endian üũ 
  - Endian�� ó���ϴ� �Լ� 
  - ���ڿ� �迭 ó�� �� 
- Packer.Read(stream, out int)



## �⺻ Ÿ�� 

Stream�� ����. Endian ó���� �Ѵ�.  

- Endian 
  - ������ little endian���� ���� 
  - Ŭ�� ����� Ȯ�� 
  - ������ ���� 
- Float / Double 
  - C++�� �޸� ������ ���缭 ��ȯ�� �����Ѱ�? 
  - �� �� �� �ִ°�? 
  - IEEE-754 ǥ�� 
    - �Ƹ��� �� �� ����
  - std::numeric_limits<float>::is_iec559 ? 
    - single precision IEEE standard 

�ణ�� ������ �ִ�. � ������ ����ϴ� ���� �߿��� ������. ��ũ��ũ �ڵ忡 ���� ������ ��ġ�� �÷��� ȣȯ������ ������ �ش�. ����̶� ���� �߿��ϴ�. 

**����:** 

- c++���� Write --> c#���� �б� 
- c#���� Write --> c++���� �б� 
- �⺻ Ÿ�ٵ� ��� �����ؼ� ó�� 
- little endian (���� ���)
- big endian���� ��ȯ �� �ٽ� ��Ʋ ����� ��ȯ �׽�Ʈ 




### ���ڿ� ó�� 

string�� char / wchar_t �迭 ���� ����.  

- char�� string ���� ó�� 
  - ���ڵ� ������ �ʿ�� �Ѵ�. 
  - ���ڿ��� ����Ʈ �迭�� ������ �� �ȴ�. 
- wchar_t�� string ���� ó�� 
  - �������� ���ڵ��� ������ ó���ؾ� �Ѵ�. 

thrift�� ��� �ϰ� �ִ°�?

- std::string, string 
- UTF8���� ó�� 
  - Encoding.UTF8.GetBytes(string)���� C#���� ó�� 
  - (uint8_t*)str.data()�� C++���� ó�� 
  - UTF8�� ���� �����ϰ� �ִ�. 

UTF8�� ����ϴ� ���� �ٶ����� ���̱� �Ѵ�. �����쿡�� �� ������ �� �Ǵ� �� �����̴�. 

string / wstring�� ���ڵ��� �¾��ϰ� �ϴ� ���� ���? �迭 ����δ� �� �� �� ������ ���δ�.  

#### char �迭 

EUC-KR�� ���ڵ��� �ְ� ó���Ѵ�.  �� ���� �����ϵ��� �ؼ� ó���ؾ� �Ѵ�. 

�ϴ�, �� �ȴ�. ������� Ȯ���� �ؾ� �Ѵ�. 

SL8, SL16 ���� Ŭ������ ���� �����ؾ� �Ѵ�. ���̸� �����ϰ� �־�� �Ѵ�. 

#### wchar_t �迭 

C#�� Unicode Encoding�� �������� UTF16�� ���ٴ� ��Ⱑ �ִ�. �׷��ٸ� �̴� �� �����ϴ�. 

�� �ʵ� �� ������ ���δ�. �׻� �׷����� �𸣰ڴ�. ����Ƽ�� �� Ÿ�� �÷������� Ȯ���� ���� �Ѵ�.  

#### ���� 

- ls8, ls16 Ÿ�� �߰�. 
- �� Ÿ�ٿ� ���� �ڵ带 �ٸ��� ����. 

### ��� 

float, double, �⺻ Ÿ�ٵ� ��ȯ�� ���� �� �ȴ�. Endian ó���� Ư���� �ʿ� ����.  �ֳ��ϸ� ��Ʈ ������ ����ؼ�   ��Ʋ ��������� ����� ������ �÷����� �������� �ʴ´�. 




## Factory 

�޼����� �ڵ带 �߰��ϰ� ������ enum ���� ���� enum �ڵ� ���ϰ� ���丮 ���, ������ �ϴ� �ڵ带 �����. �̸� ���� ã�Ƽ� �޼����� ����� Pack / Unpack�� �� �� �ֵ��� �Ѵ�. 



##  �ٱ��� ó�� 

utf8 �Ǵ� WCHAR ó��. WCHAR�� ���� ��Ŀ��� �� �� �����ϴ�. char, wchar_t Ÿ���� Ȯ���Ͽ� ó���� ����. 



# ���� �۾�



## ��� ���� 

- ���� ������ ���� 
  - include �� ���ϵ��� ���� ���� 
  - ������ �״�� ���� 



## �ʱ�ȭ �� ó�� 

InnerExpression�� double, ���ڿ�, bool ����� �� �� �ֵ��� �Ѵ�. 
�� ���� ������ Ÿ�ٿ� ���� ���� ������ ���� �����Ƿ� ���� �м����� üũ�ؾ� �Ѵ�. 
���� �������� Ÿ���� �ݿ��ϱ�� ��ƴ�. 

�� �κе� �ð� �����̴�.  ����, ������ Ȯ���� ������ ����Ѵ�. 


## ���� ���� 

�޼����� �����̶�� ���ٴ� �ʵ� ������ ���ߴ� �۾��̴�.  ��ſ� �����ϴ� ��� ���α׷��� ���� 
�ʵ���� ����Ѵٰ� �ϸ� ������ ����. ���ǿ����� �׷� ���̰� ���� �߻��Ѵ�.  

������ ���� ����� �ִ�. 

- �������� ���� ����� ��ó���� ��� ���� ������ �ǰ� �ϴ� ����� �ϳ� �ִ�. 
- define�� ����Ͽ� ���� �� ���ߴ� ����� �ִ�. 
- ��� �߰��� �ϰ� ������ ���� �ʴ� ����� �ִµ� �̴� �ʵ庰�� �ø������� �ϴ� ��츸 �����Ѵ�. 

�� ��° ����� ���ߴ� �� �ʿ��ϴ�. 

\#�� ��� �ִ� ������ �ʵ�� ó���ϰ� ���� �� �״�� �� ������ ������ִ� ����� �ִ�. 

```c++
{macro_line} {
  yylval.vline = _strdup(yytext);
  return tok_macro_line;
}
```

���� ���� �ְ� field �� �ϳ��� ó���ϰ� �ϸ� �ȴ�. 

�ʵ� serialize �ڵ忡 �ݿ��ؾ� �Ѵ�.  ���� macro_line�� ���ʷ� verbatim_block�� �����ؾ� �Ѵ�. 
verbatim block ���� �ڵ带 �����Ѵ�. ������ ������ ���δ�. 

���� ���� macro_line�� macro_line���� �̸��� �����Ѵ�. 

������ ���� ������ �� �ִ�: 
 - \#if�� if macro������ ��� 
 - \#else�� else macro ������ ��� 
 - \#endif�� ���� ���� ���� 

�� �ʵ忡 ���� ���� ���� ������ �� �����Ѵ�. �̸� ������� serialization �ڵ� ������ �ݿ��Ͽ� ó���Ѵ�. 


## include ó�� 

include ��ü�� ������� ����. ���� �ð��� ���� �ߺ� ó���� ������ ���� ���̴�. 
����, �׳� ó���� ������ ������ �ȴ�. ���� ó���ؼ� �ɹ��� �־�� �ϹǷ� �ɹ��� ������ ���� ������ �־�� �Ѵ�.
<�̸�, Ÿ��, �Ӽ�>�� ���� json ���Ϸ� ����� �̸� �ɹ� ���̺�� �ε��Ѵ�.

���� �ɹ� ���̺� json ���� ���� ��� ���� �����Ѵ�. �Ź� �����ϴ� ��
���ո����̱� �ϳ� json ���� ����� �ε��ϴ� �͵� ���̴�.
������ ������ �Ǹ� �Ź� �����ϴ� �� ���ϱ� ���� �ɹ� ������ �۾��Ѵ�. 

- \#include ��� �߰� 



## Verbatim �� 

 thrift�� doc ó��. /**���� */�����о ������ ó���ϹǷ� �̸� �����Ѵ�. 

��𿡼� ó���� ���ΰ�?  Field �տ��� lexer���� ó���ϴ� �� ���δ�. �ļ����� ó���ϴ� �� ����. 

cplus \$  VerbatimBlock \$   

���� ���·� �� �Ѵ�. �׷��� �����ϴ�. 

$�� ����ϸ� ��Ƽ���� �ּ�ó�� ó���� �����ϴ�. CaptureVerbatimBlock���� ó���� �� �ִ�. 

�������� ó���Ϸ��� }�� ������ ��Ī�� �����;� �Ѵ�. �������� �������� �ʴ�. �ϴ� $�� �����ϰ� ��� ������ ����. 

- doc text�� �̹� Ű���� ������ ������ ������ �����Ѵ�. 
- $ $ cplus �� �ؾ� �Ѵ�. 
- �ϴ�, �߰��Ǿ���. �� ������ �ϰ� �������� ���� 



## ��Ƽ���� �ּ� 

/*�� ������ */���� ������.   thrift���� �Űܼ� �߰�. 



## Ŭ���� / enum ����ȭ

/**  */���� �� definition�� �ּ� ó��. 




# C# serialize / deserialize 

- Pack(MemroyStream ) 
- Unpack(MemoryStream)
- �ִ��� �ܼ��� ����� ��� 



# expression parsing 

����� �� ��. ���� ���� ������ �۾��̴�. calculator �۾� �����ؼ� ���� 

- �迭�� ũ�⿡ �ִ� ���� ó���� ����� �� �� 


- ���� �ʱ�ȭ �� ����� ���� �� �� 
  - `i8 sv = 33` �� `byte sv`�θ� �����
  - ��� ���� �� �ȴ�. �ٿ� �ᵵ �ǰ� ������ �Ѵ�. 
- ����鿡�� +/- ������ �����ϸ� �ȴ�. 
  - �����ڷ� �������� �Ϸ��� ��� �ؾ� �ұ�? 
  - �Ǿ���. ��ū���� '+', '-'�� ����� �ļ����� ������ �߰��Ѵ�. 
    - sign �ʵ带 �߰��ϴ� �� 



## ������ �ڵ� ����

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

NetStream Ŭ������ MemoryStream�� ����ؼ� �ϳ� �����. ���⿡ ����� ó���� �����Ѵ�.  Ÿ�ٿ� ���� ������ �ϰ� Pack(), Unpack()�� ȣ���ϸ� �ȴ�. IPackable���� ��� �޵��� �Ѵ�. 

������ �ڵ带 �����ϰ� �׽�Ʈ �� �� �ű⿡ ���� �ڵ带 �����Ѵ�. �׷��� �׽�Ʈ�� �̹� �� ���̴�.  C++ �ڵ� ������ �Ѵ�.  ������ C++ �ڵ� �������� �����Ѵ�.



## C++ �ڵ� ���� 

�Լ� ������ ���� �������� �ʾҴ�.  namespace�� ���� ������ ���� ������ �ʿ��ϴ�.  

- InnerExpression���� ������ �߻� 
  - FullType '.' tok_identifier �� ����� �������� ���� 

c++ �� c# ��� namespace�� Ŭ���� ���� ������ ���� ������ ����Ѵ�. ���ƿ� ���� �ٸ���. �� ������ ������ ���� �� �����Ƿ� ���� ���ǿ� ������ �ʿ��ϴ�.  



������ �����ϰ� Pack / Unpack �ڵ�� �Ѿ��. 

- namespace, class ������ ���� �ܰ� ���� 
  - ��ͷ� ó��. �����ϰ� �� �ȴ�. �Ϲ������� ó���ϴ� �� �� ���� ���� ����. 
- C++�� Get / Set �Լ��� ���� 
- C++�� �����ڿ� C++ �� ���� 
- \# ���ε� �״�� ��� 



�̹� �� �� �����ϸ� ������ �� ������ ���δ�.  



 ## Thrift �ڵ� ���� ���� 

���� ������ �ڵ�� ��� ���⿡ ������ �ʴ�. partial Ŭ������ �߰��Ͽ� �ٸ� ������ ���ø����̼ǿ��� �ʿ��ϰ� ���� �����ϴ�. 

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



# C\# �ڵ� ���� ���� �۾� 

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



- �ʵ带 public���� �����. property? 

- Ÿ�ٵ� ���߱� 

  - �⺻ Ÿ�ٵ� ��ȯ 

    - �� bool, short, int, long, float, double
    - C#���� int�� Int32, long�� Int64�� �� 

  - �迭 

  - Ŭ������ enum ���� 

    - �ɹ� ���̺��� ã�Ƽ� �����ؾ� ��


# C\# �ڵ� ���� 

 - ��� ���� 
   - node�� ���� ���� 
   - �����ϸ鼭 ������ ���� 
   - namespace�� ������ c# �ڵ带 ������ �� ���� 
     - json���� �⺻ ������ ���� ������ �ʿ� 
	 - �ϴ��� �����ϰ� �����ؼ� json���� �ű� 

 - ���α׷� ������ ����
   - ���۰� ���� (���)
   - ���� 
   - ��� (include) 

 - ���� serialization �ڵ� ���� 
   - simple 
   - full 

 - Expression �Ľ��� �� �� 
   - operator�� �ν����� �� �� 
   - �Ѵٰ� �ص� ������ �� ���� ���� 
   - �ϴ�, �⺻ ��� ������ �����ϰ� ó�� 
   - ���� �ܰ� namespace ������ �Բ� ó�� 
   

# Code cleanup 

 - check pointers 
 - delete pointers (destructor)

# Debugging AST 

 - g_program���� ���¸� �ľ��Ѵ�. 

 - idl_parser.yy�� �극��ũ�� �ɾ�� �����Ѵ�. 
   - \#line ������ �׷���.  

# C\# �ڵ� ����


C# �ڵ� ������ ��ǥ�� AST�� �����ϰ� generator �۾��� �����Ѵ�.  

idl_program�� ������ ���� �־�� �ϴ°�? 

- TypeDefinition�� 
  - enum, struct, message 
  - ������ ���� �ʿ� 
  - ������ ���ؼ��� ��ü include�� ���ϵ��� �ɹ��� �ʿ� 


- include ó�� 
  - idl_program���� parsing ���� 
  - symbol table�� �Բ� ��� 
    - namespace ����� ���� 

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

��κ��� ����� ���� �����̴�. ���� ���� ��ȿ�� c++ �ڵ��̸� c#�� pack/unpack �ڵ常 �� ��������� �ȴ�. ���� �������� �׽�Ʈ�� �����Ѵ�. 

## Semantic Actions 

- enum 
- struct 
- field 
- simple / full type
- array 

###���� 

Node ������� Ʈ���� �����ϰ� ������ �� �ƴϸ� thrift ó�� �׳� program�� Ÿ�ٺ��� �߰��ؼ� ������ �� ������ �� �ؾ� �Ѵ�. 
Node������� �ϸ� �ٸ� �ļ� / ������ �ۼ��� �� ������ ������ ������ �� �ִ�.  
thrift�� ������ ������ Ʈ���� �������� �ʰ� ���信 �°� ���� �����ߴٴ� ���� �ٸ���. 

����, Node�� �ڽ����� ������ ����� �� ���� �� �ϴ�. 

bison�� c++ ������ �ۼ��� ����� ������ AST�� Ʈ���� �����ϴ� ���� �ۼ������Ƿ� �̿� �����Ͽ� �����Ѵ�. 

- idl_node
  - print() = 0;
  - indent
  - children
- idl_node_enum 
  - enum_value 
- idl_node_struct 
  - field 

symbol_table�� ���´�. namespace�� ������ full_name�� Ű�� �ȴ�. ���� ������ namespace�� ���δ�.  
������ ���� ���� ���� ũ�� �ǹ̰� ��������. �� �� ������ �޴� �κ��� ����. 
������ Syntax Directed Translation�� �� �����ϴ� �� �� �߿��ϴ�.

## enum 

- new �ϴ� ���� 
- append�� ���� �߰� 
- program �߰� ���� 

thrift �� ������ ������ �ȴ�. SimpleExpression�� ó���� ���� �ٸ���.  




## struct 

���� ���ϸ� ���� �� �ϴ� ���̴� �� ���� ���� �����Ѵ�. 

- �ֻ������� �����Ѵ�. 
- FieldList�� ���� ���´� empty ��Ģ�� ���� ���� ����ȴ� 

```c++
SimpleExpression: 
   SimpleExpression '+' SimpleExpression
	{
		util::log()->debug("SimpleExpression => SimpleExpression '+' SimpleExpression");

		// shift / reduce conflict�� ������ ������. 

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

�� ��ٷο� �����̴�. ���չ������� add_expression()�� �Ѵ�. FullType ��� expression�� �����. 

### Symbol Validation 

�ڵ� ������ �� ���������� �Ľ��� �� �̸� ������ �߰��� �� �ִٸ� ����. symbol_table�� ����� �ؾ� �ϴµ� ������ ������ ������� �ľ��ؾ� �Ѵ�. ���� �ø�ƽ �׼��� �����ϵ��� �� ���� �ڵ带 �߰��ϴ� �� ���� �� �ϴ�. 

## AST ���� 

enum / struct�� ���� �߰��ߴ�. ������ ������� �ʴ�. �׷���, �ֻ����� program���� �����ؾ� �ϴ� �͵��� node�� �Ѵ�. 

- idl_node
  - idl_node_struct 
  - idl_node_enum 
  - idl_node_include 

Ÿ������ ó���Ǵ� �͸� type���� �Ѵ�. 

- idl_type 
  - idl_type_simple
  - idl_type_full

�������� struct, enum, message � �߰��ǹǷ� �׳� �̸��� ����. 

- idl_enum_value 
- idl_field
- idl_field_value
- idl_expression 

expression�� ó���ϰ� ������ϸ� ���� �ڵ� �������� �Ѿ �� �ִ�.  

���� �׽�Ʈ�� �����ϰ� �ڵ� ������ �ϸ鼭 �� ���ɴ�. expression�� ���� ����̱� �ϴ�.  CSP�� �����ϸ� �ǹ̷п� ���� �� �ٰ��� �� �ִ�. 



# �ΰ�  

�ΰŸ� �ٿ��� ó���Ѵ�.  ������ ����. �ٸ� �͵��� semantic action�� �߽����� ó���ǵ��� �Ѵ�. 


# �ּ��� ó�� 

{comment}�� lex ��Ģ�� �߰��Ͽ� ó��. c �ּ��� �ϴ� �������� ����. 



# ������ expression ó��

Hello.v1 + 3*2 

���� ���� ������ ������ ó���� �ʿ��ϴ�. enum�� tok_int_constant ��� ����� �� �־�� �Ѵ�. enum ���� ������ �����Ѵ�. 

SimpleExpression: 

   SimpleExpression '+' SimpleExpression

| SimpleExpression '-' SimpleExpression 

| FullType '.'  tok_identifier 

| tok_identifier 

| int_constant 

���� ������ ����� ���δ�. AST ���� �� �� �� �ٵ뵵�� �Ѵ�. 

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

���� ���ǿ��� ���� shift / reduce conflict�� �߻��ߴ�.  

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

�� ���ϱ�, ���� ��ȣ���� shift�� reduce�� ���ÿ� �߻��Ѵ�. bison�� �̸� ó���ϱ� ���� shift�� ���� �ϵ��� �Ǿ� �ְ� (�� �� ���̸� �Ľ��ϰ� �ȴ�) []�� ���� �κ��� ���õ� reduce�� ǥ���Ѵ�. �����̴�. 



# ���ӽ����̽� ó�� 

SimpleType �ܿ� FullType ���� namespace�� ���� ���� ������ �����ϵ��� �Ѵ�. 

�����ϰ� ó���ϱ� ���ٴ� �Ѱ踦 �δ� �� ���� �� �ϴ�. 

```
FullType: 
  tok_identifier tok_namespace_separator tok_identifier 
	{
	}
```

���� ���Ƿ� �� �ܰ� namespace�� �����Ѵ�. FnApp::Struct1�� ���� ���¸� �����Ѵ�. 



# Flex / Bison ��� �ļ� 

�ܰ������� �����Ѵ�. C ��Ÿ���� ���� / �ļ��� ����Ѵ�. (thrift�� �̿� ����)

## enum 

enum identifier { 
	age = 3, 
	value = other_enum + 100, --> ���߿�. thrift�� const ���� �����ϰ� �Ǿ� ����. Ȯ���� �ʿ��� ���� �� �ϳ�
};

### thrift�� ���� 

- %type<tenum>     Enum
- %type<tenum>     EnumDefList
- %type<tenumv>    EnumDef
- %type<tenumv>    EnumValue

## ������ ���� 

idl_parser.yy���� idl_parser.cpp�� idl_parser.hpp�� ����. 

idl_lexer.ll���� idl_lexer.cpp�� ���� 

idl_main.cpp�� parse() �Լ����� �Ľ��� �����Ѵ�. 


## Rule�� ó�� 

- ��Ī �Ǿ��� �� ó�� �ൿ�� ����
- ���� new�� �����ϴ� ��?
- $$, $1���� ��� �����ϴ� ��?

�� �κ� �ذ��ؾ� �ڿ������� ó���� �����ϴ�. 

- AST �����ϴ� ��� �����̴�. 
- $$�� Ÿ���� ��Ȯ�ϰ� ���� �ȴ�. (group �Ǵ� nonterminal�� Ÿ��)
- $1���� \$n������ �׷��� �ȴ�. 



## enum 

��� ���� ������� ���� --verbose�� --report�� �߰��ϸ� .output ������ �����ȴ�. �ڵ� ������ ������ �����ϹǷ� ��� ���� ���� ���׸� ã�� ����. 

- enum �ν� �� �ϴ� ���� 
  - �տ� identifier�� �־ �ν� �� �� ������ ���� 
  - ���ڿ� ��Ī�� ���� ������ �ø��� ���� ���� 
- v1 (identifier) ���� 
  - identifier�� ��Ī �� �Ǵ� ������ ���� 
- �ļ� ���� ����� 
  - YYDEBUG �� 1�� ���� 
  - yydebug ���� 1�� ���� 
  - ���� ����, ��ū Ÿ���� ����. 
  - ���� �ÿ��� �׻� �Ѱ� ���� �� �� 

### enum identifier { identifer }���� { �ν��� �� �� 

'{' ���� �ļ��� ���ǵǾ� �ִµ� identifier �ν��� �� ��. 

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

state 5���� state 7���� �Ѿ�� �� ������ �ʴ´�. enum�� enum�� identifier�� �а� { ���� ���� ������ �Ͼ�� �ʾҴ�. �� �׷���? 

 ```
State 5

    3 Enum: tok_enum tok_identifier . '{' EnumDefList '}'

    '{'  shift, and go to state 7
 ```

state 5�� '{'�� ������ state 7���� ���� �Ǿ� �ִ�. �׷��� ���� �ʾҴ�. %error-verbose�� yy���Ͽ� �߰�. ���� �޽����� `syntax error, unexpected tok_identifier, expecting '{'`�� ����. '{'�� ��򰡿��� ����� ������ ���δ�. 

��� �����Ǵ� �� ������� ���� �ʴ�. 

tok_open_brace, tok_close_brace�� �߰��ϰ� ��������. ������ = ���ڿ��� ������ �߻��ߴ�. ��ū���� ����ϱ� �� �ȴ�. �� ������ ��������? 

�߰�ȣ�� ������� �ؼ� ������ ã�´�. 

**�����:** 

- ���� �ɼ��� �ִ� ���ΰ�? 
  - �ɼ� ������ �ƴ� �� �ϴ�. 
- �⺻ �� (lexer�� default ��)�δ� ��ū���� �������� �ʴ´�. 
- symbol�� ��� ���� �ذ� (thrift���� �׷��� �̹� �Ǿ� �ִµ� �����ؼ� ���� ����)

### ������ �ܴ��ϰ� ����� �����ϱ� 

- ����� ������ ����� 
  - %error-verbose�� yydebug ��ũ�� ������� ��� ���� ���� �� 
  - lexer �� ��ū ����ϰ� ����� 
    - ������ ��� ��Ī�Ǿ��� �� �˱� ��ƴ�. 
    - �̸� ����� �����ϰ� �Ѵ�. 

flex -d --wincompat -o "idl\\idl_lexer.cpp" idl/idl_lexer.ll && bison -y -d --verbose -o "idl\\idl_parser.cpp" idl/idl_parser.yy

- flex -d �ɼ��� ������� �����ϰ� �Ѵ�. 
- --verbose ���� output ������ ����� �ļ� ������ �� �� �ְ� ���ش�. 
- -y �ɼ��� yacc ȣȯ����̴�. 

flex -d�� --verbose, YYDEBUG ������� ����� �� ������� �� �ְ� �Ǿ���. �Ľ� ���� ��ü�� ���ϰ� �����ϸ� �� ������ ���̴�.  ���ش� �����Ϸ��� �ø�ƽ���� ���̴�. 

## struct

�⺻ �Ľ��� ���� ����Ǿ���. �߰� ����� ������ ����. 

- �Ϲ� Ÿ���� ó�� 
  - namespace ó�� 
- �迭 Ÿ���� ó�� 
  - expression�� ���. 
    - enum �� ���� ��� ���� ���� 
    - enum������ ����� �� �ִ� expression 



## message 

����� ����μ��� �ǹ̰� ������ struct�� �׽�Ʈ �ϸ� �ȴ�. 





## �Ľ� Ʈ���� �ɹ� ���̺� 

 - Type 
   - EnumType 
     - EnumFieldType
   - StructType 
   - MessageType
   - FieldType 
     - PrimitiveType
	 - ArrayType
   - ConstType

## �����ذ� 

 - expression ����
   - assignment 
     - enum value 
	 - default value
   - array



## �ڷ� 

 - thrift �ڵ带 �ַ� ���� 
 - https://www.lysator.liu.se/c/ANSI-C-grammar-y.html
   - C ���� ���� ���� 



# �ܰ����� �ļ� / ������ ���� 

 - C# �ڵ� ������ ���� ���� 
 - Pack(Stream), Unpack(Stream) �������̽��� ���� 
 - ���� ��������� �켱 ���� 
 - ���ڵ��� ������ ó�� 


## vs2015 �̽� 

spirit x3�� ����� �� decltype�� ���� �߷п��� ������ �߻��Ѵ�. vs2017������ ����� �����Ѵ�. 
��ó�� c++�� template�� Ÿ�� ���� ��� ������ ��Ȳ�� �����ð� �����. 
���� ���۾����� �Ľ��� �ϴ°� ���� ���� �ִµ� ������ ������� �ʴ�. 
flex�� bison�� ����ؼ� ���������� ����� �� ���� ���� �ִ�. 

qi�� x3�� ������ ���� ����� �ڵ��� �� ������. x3�� rule�� attribute ������� ast�� �� ����� �ش�. 
�� ������ �����ص� �ǰ����� ���� ���� ������ ���� �� �ִ�. ����, flex, bison�� �ٽ� �õ��Ѵ�. 


## �۾� ���� 

 - enum 
 - struct
   - variable_decl  
 - message   
 - namespace 
 - include 


## c# �ڵ� ���� 

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
   - ���̸�ŭ ��ȭȭ�鼭 Pack/Unpack ȣ�� 
   - Ÿ���� struct / enum / primitive / ���ڿ� ���� 

### ���ڿ� 

 ���ڵ� �ؼ� �迭 ����. 



# project setup 

- �⺻ ��ƿ��Ƽ �߰� 
  - �α� (spdlog) 
  - ��ũ�� 
  - exception 

- ��� �߰� 
  - BOOST_HOME �ʿ� 


# ����� option ó�� 

- boost.program_options ��� 
  - http://www.boost.org/doc/libs/1_58_0/doc/html/program_options/tutorial.html
  - ��� �ɼ� 
    - cplus / cpp 
	- csharp
  - ���� �ɼ� 
    - force
  - ���� üũ�� 
    - parse 
	- �Ľ� Ʈ�� ��� 
	- ���� ������ ��ü ���� 
  - �ʿ�� �߰��ϸ鼭 ���� 
    

# �Ľ� ���� ��°� ���� 

��ü ���� �������� ������ ���� �ʿ�. 
json �������� ����. 

�� AST �׸��� ���Ϸ� ����� ���������ν� ��ü ȿ���� �ø�

# �ļ�

- enum �ļ�
- struct / message �ļ�
  - c# �ڵ� ����
  - (de)serialization �׽�Ʈ
- c++ �ڵ� ����
  - (de)serialization �׽�Ʈ
- ���� �ڵ鸵�� �����ϸ鼭 �߰� �߰� �߰�

- include ó��
- namespace ó��
- using ó��

- ���� ��� �߰�
  - ��� ���� �������� �� �� ����
  - flatbuffers�� �������� ��
    - enum ������ �ִ�. 
	- r2c�� �߰� ������ �� �ʿ��ϴ�. 
	

# ������ 

spirit�� boost�� �����ϰ� R2 �ڵ�� �����ϱ� ���� boost convention�� ����Ѵ�. 

- ��� �̸��� �ҹ��ڿ� ����ٸ� ��� 
  - ��ũ�δ� �빮�� ��� 
  - ���ø� �ƱԸ�Ʈ�� PascalCase ��� 

- �ּ� 
  - �ڵ尡 ������ �ǵ��� �߿��� ����鿡 ���� �ּ� �ޱ� 
  - ���� �� ����� �κп� ���� �ּ� �ޱ� 



# ���� �� ���� 

## �ļ��� 

���� �ļ������ �����ϰ� �ڵ� �������� ����. �׽�Ʈ �� ���� �ܰ�� ����. 



# Reading Manual 

�� ó�� �κ��� �����ϱ� ���� ������� ���� �ڵ� �帧�� �ľ��ϰ� ������ �����ؾ� �Ѵ�.  bison �Ŵ����� �����鼭 ���󰣴�. 

- LALR(1), GLR parsing 
- token / grouping for terminal and non-terminal symbols 
  - token���� �׷��� ����� �۾�
  - bottom-up (LR) parser�� �̷��� �����Ѵ�. 
- token type and semantic value 
- semantic actions 
  - executed on a match 
  - get semantic value from the semantic values of its parts
- GLR�� ���� ���� 
  - ���� ���� �ļ����� ����� �����ؼ� �����ϴ� ���� 
  - ��� ���ǿ��� �����ϴ� �� ���� 
  - �ʿ��� ��찡 �ֳ� ���� ���߿� �� �� 
- @$, @1, @3 
  - �����̼� �����ؼ� Ư���� �� ���� ����. 
  - semantic action�� ��� ���� ����� �� �ʿ� 

## Bison Grammar Rules 

### Outline 



- Prologue 
- Bison declarations 
- Grammar rules 
- Epilogue 

#### Prolog 

���� �� �� �� �ִ�. YYSTYPE�� %union�� Ÿ���� �ǹ��Ѵ�. 

%code�� ��ü�� �� �ִ�. top, requires, default (��). %code top { }, %code requires {} 

requires�� ��� ���Ϸ� ���ų� ���� ����. 

### Symbols, Terminal and Nonterminal 

��ū���� ������ ���� ���ǵȴ�. 

- named token type 
- character token type 
- literal string token 

-d �ɼ����� �����ϸ� ��ũ�� ���Ǹ� ����� �߰��Ͽ� lexer���� ��� �����ϵ��� �Ѵ�. 

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
  - ��κ��� ��� ��� �� �ϴ� �ϴ� ��ŵ

### Tracking Locations 

- YYLTYPE ����. 
- @$�� ����. @1, @3 ������ ���� 

### Named References

�̸��� ����ϰų� �̸��� �� �� �ִ�. 

```c++
invocation: op ��(�� args ��)��
{ $invocation = new_invocation ($op, $args, @invocation); }
```

ȥ���� ���� ��� exp[left]�� ���� �� �� �ִ�. 



### Bison Declarations 

- token type names
  - %token name 
  - or %left, %rifht, %precedence, %noassoc 
  - %token <Type> ID 
    - Ÿ���� �ֱ����� ��� 
- nonterminal symbols (grouping)
  - %type <Type> nonterminal 
- %destructor 
  - �޸� ������ ��� 
  - ��ġ �ļ������� ũ��Ƽ���� ������ �ƴ� 
  - thrift�� ���� �߻��� ���� �ϴ� ������ �� 


# Syntax Directed Translation 

�Ľ� Ʈ���� �� ��忡 �Ӽ� ���� �ְ� �̸� ������� �ǹ� �ؼ��� �����Ѵ�. 

- attribute grammar 
  - attributed attached to syntax
  - each production in a grammar, give semantic rules or actions
- inherited attribute 
  - semantic rule associated with the production at the parent of N. 
  - attribute values at N's parent, N itself and N's siblings
  - Ÿ�� ��� ���� ���� ���� 
- synthesized attribute 
  - �� ��忡 �־��� �ϳ��� �Ӽ� �� 
  - attribute values at children of the node 
  - attribute value at node itself 
  - S-attributed definitions 
    - bottom up�� ����
- annotated parse tree
  - ���� �Բ� ���� �ִ� Ʈ�� 
- L-attributed definitions
  - ���ʿ��� ���������� ��Ʈ���� ���� 
  - ��Ӱ� �ռ� �� ��θ� ��� 

SDT�� SDD�� �ٸ���. ���� ������ SDD�� �ش�. SDT�� ���� ���� ��Ģ�� ������ ���� ��. bison�� ���� ����� �̰��̴�.  
SDT�� SDD�� �����ϰ� L-attributed SDD�� ����Ѵ� (preorder action traversal). bison�� synthesized attribute�� �ַ� ����Ѵ�. 
�ֳ��ϸ� \$1, \$n�� �ڽ� ����̱� ������ �ڽ� ����� ������ ������ �� �ִ�. 


# TODO

- c# �ڵ� ����
  - �⺻ Ÿ�ٵ�
  - ���ڹ迭
  - �迭
- c# �ڵ� �׽�Ʈ 
- c++ �ڵ� ������ �׽�Ʈ 
  - c#�� �ſ� �����ϴ�. 
- include
- message
- ���� ����  
  - ��� �� ���ΰ�? sequence�� �߰��� �����ϰ�? 
- ���� ���� 
  - double, float �ʱ�ȭ �� ���� �����ϰ� �� 
  - namespace ��� �� ���� �� �����ϰ� ��  OK
- �ɹ� üũ 
  - c#�� ��� ��ü ������ �����Ͽ� ������ �Ұ����ϴ�. 
  - c++�� ���� �����ϴ�. ���߿� �����Ѵ�. 
- ���� �ڵ� �ߺ� 
  - ����� �κ��� ������ �̸� ��� �� �� ����ؾ� �Ѵ�. 
  - �ߺ� ���Ŵ� ���� ���α׷��� �����̴�. 
- verbatim �� �߰� OK
  - cplus { }, csharp {} ���·� ��� 
  - �ּ��� ó�� ������ ���캸�� �۾�
- namespace OK
- AST ���� OK
  - Seems  
  - Add cleanup code
  - Add check code 
- message node �߰� OK
  - ���⸦ ���� ������ �Ѵ�
  - �ϴ�, struct�� �����ϰ� ����
  - c++�� c# ��� �߰� �ڵ� ������ �ʿ��ϴ�
- c++ ���� �Լ� ���� 
  - �ʵ�� private�� ���� 
  - public �Լ��� ���� 
  - Ÿ�ٿ� ���� ������ ó���� ���� 

