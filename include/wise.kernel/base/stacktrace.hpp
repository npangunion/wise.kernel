#pragma once

#include <string>

namespace wise {
    namespace kernel {
        /// boost�� ����Ʈ���̽��� �α� ���
        /**
         * NOTE: MSVC �� ���� ��ũ�� ���� ����ȭ ������ �������� ����. 2018-11-07.
         */
        class stacktrace
        {
        public:

            static std::string dump(const char* msg);
        };
    } // kernel
} // wise