#pragma once

namespace wise {
    namespace kernel {
        /// boost�� ����Ʈ���̽��� �α� ���
        /**
         * NOTE: MSVC �� ���� ��ũ�� ���� ����ȭ ������ �������� ����. 2018-11-07.
         */
        class stacktrace
        {
        public:

            static void dump(const char* msg);
        };
    } // kernel
} // wise