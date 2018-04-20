/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <graphene/chain/protocol/operations.hpp>


namespace graphene { namespace monitor {

    using namespace chain;

    class operation_monitor
    {
    public:
        operation_monitor(){};
        virtual ~operation_monitor(){};

        virtual void monitor_op(const operation& op) = 0;

        virtual void dump_stat() = 0;
    };

    template<typename OpMonitorType>
    class operation_monitor_imp : public operation_monitor
    {
    public:
        operation_monitor_imp(){};
        virtual ~operation_monitor_imp(){};

        void monitor_op(const operation& op) override
        {
            m_opMonitor.monitor_op(op);
        }

        void dump_stat()
        {
            m_opMonitor.dump_stat();
        }

    private:
        OpMonitorType m_opMonitor;
    };

    template<typename DerivedMonitor>
    class op_monitor
    {
    public:
        op_monitor(){m_periodStartTime = fc::time_point::now().sec_since_epoch();};
        virtual ~op_monitor(){};

        virtual void monitor_op(const operation& o)
        {
            auto* monitor = static_cast<DerivedMonitor*>(this);
            const auto& op = o.get<typename DerivedMonitor::operation_type>();
            return monitor->do_monitor(op);
        }

        virtual void dump_stat() = 0;

        virtual bool start_new_period()
        {
            uint32_t now = fc::time_point::now().sec_since_epoch();
            if (now - m_periodStartTime > m_period)
            {
                m_periodStartTime = now;
                return true;
            }
            else
            {
                return false;
            }
        }
    private:
        uint32_t m_periodStartTime;
        uint32_t m_period = 5*60;
    };

} } 
