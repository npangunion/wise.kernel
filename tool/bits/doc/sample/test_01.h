#pragma once
#include <stdint.h>

#pragma pack(push, 1)

namespace Test{
namespace Message{

enum Hello
{
	V1,
	V2,
	V3 = 97,
};


class Test
{
public:
// field functions
const int32_t GetV1() const;
const double GetFf() const;
const Hello& GetHv() const;
const na::h& GetCan() const;
const na::h* GetArr() const;

private:
 // field variables 
	int32_t	__mV1 = 33;
	double	__mFf;
	Hello	__mHv;
// $ 
 
	void HelloCFunction();	
	
// $ 
	na::h	__mCan;
	na::h	__mArr[Hello::v3];
};


class Level2
{
public:
// field functions
const Test& GetField1() const;
const Self::Hello::Test* GetField2() const;

private:
 // field variables 
	Test	__mField1;
	Self::Hello::Test	__mField2[2];
};


class ReqMove
{
public:
// field functions
const FnlFw::CUnique& GetWho() const;
const FnlApp::T3D& GetPos() const;
const TPosDir& GetDir() const;
const int32_t GetHp() const;

private:
 // field variables 
	FnlFw::CUnique	__mWho;
	FnlApp::T3D	__mPos;
	TPosDir	__mDir;
	int32_t	__mHp;
};

} // Test
} // Message

#pragma pack(pop)

