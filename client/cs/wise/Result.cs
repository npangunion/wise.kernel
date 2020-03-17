using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{
    public class Result
    {
        public enum Code
        {
            Success,
            FailConnectTimeout,
            FailConnectException,
            FailListeningException,
            FailSocketDisposed, 
            FailSessionException, 
            FailTooLargeMessage,
            FailMessageNotRegistered, 
            FailMessagePack,
            FailMessageUnpack,
            FailInvalidRecvSequence,
            FailInvalidCrc,

            /// and others
        }

        private Code code = Code.Success;

        public static implicit operator bool(Result result)
        {
            return !object.ReferenceEquals(result, null) && result.code == Code.Success;
        }

        public static Result Set(Code code)
        {
            var result = new Result();
            result.code = code;
            return result;
        }

        public static Result Success()
        {
            return Set(Code.Success);
        }

        public static Result Fail(Code code)
        {
            return Set(code);
        }

        public override string ToString()
        {
            return string.Format("Result: {}", code.ToString("g"));
        }

    }
}
