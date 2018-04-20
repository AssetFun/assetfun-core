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
#include <graphene/monitor/witness_monitor.hpp>

namespace graphene { namespace monitor {

using namespace chain;

uint32_t witness_monitor::witness_active_time_guard = 300;

void witness_monitor::monitor_object(const database& db)
{
	//[lilianwen add 2018-1-20]判断见证人是否离线
	const dynamic_global_property_object& dpo = db.get_dynamic_global_properties();
	const global_property_object &po = db.get_global_properties();

	auto ite_witness_id=po.active_witnesses.begin();
	for (;ite_witness_id != po.active_witnesses.end(); ite_witness_id++ )
	{
		const auto& idx = db.get_index_type<witness_index>().indices().get<by_id>();
		object_id_type id = *ite_witness_id;
		auto itr = idx.find(id);
		if( itr != idx.end() )
		{
			const witness_object &wo = *itr;
			if( dpo.head_block_number - wo.last_confirmed_block_num > witness_active_time_guard)//距离上次出块已经有这么久没有出块了
			{
				m_witnessState[id]=false;
			}
			else
			{
				m_witnessState[id]=true;
			}
		}
		else
		{
			monitor_elog("witness[${witness_id}] not found.", ("witness_id", id));
		}
	}
	
}

void witness_monitor::dump_stat()
{
    ilog("------------------- witness_monitor::dump_stat begin -------------------");
	for(const auto &one:m_witnessState)
	{
		if (one.second)
		{
			ilog("i Witness[${witness_id}] is on-line.", ("witness_id", one.first));
			//wlog("w Witness[${witness_id}] is on-line.", ("witness_id", one.first));
			//elog("e Witness[${witness_id}] is on-line.", ("witness_id", one.first));
			//monitor_elog("Witness[${witness_id}] is on-line.", ("witness_id", one.first));
		}
		else
		{
			monitor_elog("Witness[${witness_id}] may be off-line.", ("witness_id", one.first));			
		}
	}
 
    ilog("------------------- witness_monitor::dump_stat end -------------------");
}

void witness_monitor::set_witness_active_time_guard(uint32_t guard)
{
    witness_active_time_guard=guard;
}

} }
