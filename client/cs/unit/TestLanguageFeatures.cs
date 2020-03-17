using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Security.Cryptography;

namespace unit
{
    class KeyObject
    {
    }

    class Cipher
    {
        private static readonly string key = "2B7E151628AED2A6";
        private static readonly string iv  = "ACE03D519F3CBA2B";

        class Encryptor
        {
            AesCryptoServiceProvider dataEncrypt;
            ICryptoTransform crypto1;

            public Encryptor()
            {
                dataEncrypt = new AesCryptoServiceProvider();

                dataEncrypt.BlockSize = 128;
                dataEncrypt.KeySize = 128;
                dataEncrypt.Key = GetKey();
                dataEncrypt.IV = GetIV();
                dataEncrypt.Padding = PaddingMode.PKCS7;
                dataEncrypt.Mode = CipherMode.CBC;

            }

            public byte[] Encrypt(string input)
            {
                crypto1 = dataEncrypt.CreateEncryptor(dataEncrypt.Key, dataEncrypt.IV);

                byte[] inputBytes = Encoding.UTF8.GetBytes(input);
                byte[] encryptedData = crypto1.TransformFinalBlock(inputBytes, 0, inputBytes.Length);

                crypto1.Dispose();

                return encryptedData;
            } 
        }

        class Decryptor
        {
            AesCryptoServiceProvider dataDecrypt;
            ICryptoTransform crypto1;

            public Decryptor()
            {
                dataDecrypt = new AesCryptoServiceProvider();

                dataDecrypt.BlockSize = 128;
                dataDecrypt.KeySize = 128;
                dataDecrypt.Key = GetKey();
                dataDecrypt.IV = GetIV();
                dataDecrypt.Padding = PaddingMode.PKCS7;
                dataDecrypt.Mode = CipherMode.CBC;

            }

            public string Decrypt(byte[] input)
            {
                crypto1 = dataDecrypt.CreateDecryptor(dataDecrypt.Key, dataDecrypt.IV);

                byte[] decryptedData = crypto1.TransformFinalBlock(input, 0, input.Length);

                crypto1.Dispose();

                return Encoding.UTF8.GetString(decryptedData);
            }
        }

        static Encryptor encryptor = new Encryptor();
        static Decryptor decryptor = new Decryptor();

        public static byte[] Encrypt2(string input)
        {
            return encryptor.Encrypt(input);
        }

        public static string Decrypt2(byte[] input)
        {
            return decryptor.Decrypt(input);
        }

        public static byte[] Encrypt(string input)
        {
            Aes rm = Aes.Create("AesCryptoServiceProvider");
            // 알고리즘 간 차이는 뭘까? AesManaged, AES, AesCryptoServiceProvider 등

            rm.Mode = CipherMode.CBC;
            rm.Padding = PaddingMode.PKCS7;
            rm.KeySize = 128;
            rm.Key = GetKey();
            rm.IV = GetIV();

            ICryptoTransform encryptor = rm.CreateEncryptor(GetKey(), GetIV());

            byte[] inputBytes = Encoding.ASCII.GetBytes(input);

            MemoryStream ms = new MemoryStream();

            using (CryptoStream cs = new CryptoStream(ms, encryptor, CryptoStreamMode.Write))
            {
                using (StreamWriter cw = new StreamWriter(cs))
                {
                    cw.Write(inputBytes);
                }
            }

            return ms.ToArray();
        }

        public static string Decrypt(byte[] input)
        {
            Aes rm = Aes.Create("AesCryptoServiceProvider");
            // 알고리즘 간 차이는 뭘까? AesManaged, AES, AesCryptoServiceProvider 등

            rm.Mode = CipherMode.CBC;
            rm.Padding = PaddingMode.PKCS7;
            rm.KeySize = 128;
            rm.Key = GetKey();
            rm.IV = GetIV();

            ICryptoTransform decryptor = rm.CreateDecryptor(GetKey(), GetIV());

            // MS 샘플은 using을 사용. Dispose가 GC 부담을 더는 한 방법
            // CryptoStream이 Dispose 될 때 뭔가 처리를 한다. using 안 쓰면 안 됨

            string plainText = "";

            using (MemoryStream ms = new MemoryStream(input))
            {
                using (CryptoStream cs = new CryptoStream(ms, decryptor, CryptoStreamMode.Read))
                {
                    using (StreamReader rc = new StreamReader(ms))
                    {
                        plainText = rc.ReadToEnd();
                    }
                }
            }

            return plainText;
        }

        private static byte[] GetKey()
        {
            return Encoding.UTF8.GetBytes(key);
        }

        private static byte[] GetIV()
        {
            return Encoding.UTF8.GetBytes(iv);
        }
    }

    [TestClass]
    public class TestLanguageFeatures
    {
        [TestMethod]
        public void TestObjectKeyDictionary()
        {
            var dict = new Dictionary<KeyObject, int>();

            var k1 = new KeyObject();
            var k2 = new KeyObject();

            dict.Add(k1, 3);
            dict.Add(k2, 4);

            int val;
            Assert.IsTrue(dict.TryGetValue(k1, out val));
            Assert.IsTrue(val == 3);

            Assert.IsTrue(dict.TryGetValue(k2, out val));
            Assert.IsTrue(val == 4);
        }

        [TestMethod]
        public void TestMemoryStreamPosition()
        {
            var ms = new MemoryStream();

            ms.WriteByte(3);
            ms.WriteByte(2);
            ms.WriteByte(1);

            Assert.IsTrue(ms.Position == 3);

            ms.Position = 0;

            int v1 = ms.ReadByte();
            int v2 = ms.ReadByte();

            Assert.IsTrue(v1 == 3);
            Assert.IsTrue(v2 == 2);

            ms.Position = 2;
            int v3 = ms.ReadByte();
            Assert.IsTrue(v3 == 1);

            // Position은 읽고 쓸 때 모두 사용 
            // 쓰기와 읽기를 구분하여 Position을 사용해야 함
        }

        [TestMethod]
        public void TestEncryptionAlgorithm()
        {
            var enc = Cipher.Encrypt2("Hello World!!?");
            var dec = Cipher.Decrypt2(enc);

            Assert.IsTrue(dec == "Hello World!!?");

            enc = Cipher.Encrypt2("Hello World!!?");
            dec = Cipher.Decrypt2(enc);

            Assert.IsTrue(dec == "Hello World!!?");

            // 패딩 자동 처리됨
            // Encrypt 함수 동작 안 함. 
            // - 디버깅할 필요가 있을 지.... 
            // - 잘 안 되니 동작하는 버전을 기초로 진행한다.

            // TransformBlock과 TransformFinalBlock 
            // - 서버의 process / finish 구조와 같은 것???
            // 

            // 
            // ICryptoTransform이 여러 번 사용 가능하다고 하는데 
            // 동일한 호출에 대해 같은 암호화를 돌려주고 
            // 패딩 오류가 나온다. 왜 그런가? C++과 맞출 수 있을까? 
            //

            // 
        }

        [TestMethod]
        public void TestSHA1()
        {
            byte[] data = Encoding.UTF8.GetBytes("Hello SHA1");

            SHA1 sha = new SHA1CryptoServiceProvider();

            var result = sha.ComputeHash(data);
            var hash = wise.Detail.CipherModifier.BytesToHexString(result);

            Assert.IsTrue(hash == "6130AD8CA215EDDAAA9A3004DAB3EB0525DE2D63");
        }
    }
}
