#pragma once

namespace wise {
    namespace kernel {
        /// boost의 스택트레이스로 로그 출력
        /**
         * NOTE: MSVC 더 빠른 링크를 위한 최적화 켜지면 동작하지 않음. 2018-11-07.
         */
        class stacktrace
        {
        public:

            static void dump(const char* msg);
        };
    } // kernel
} // wise