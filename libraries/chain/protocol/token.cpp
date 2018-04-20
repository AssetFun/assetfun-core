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
 * furnished to do so, token to the following conditions:
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

#include <graphene/chain/protocol/token.hpp>

namespace graphene { namespace chain {


template<typename SubjectTemplateType>
static SubjectTemplateType token_template_check(const variant& from)
{
	SubjectTemplateType result;
	from_variant(from, result);
	return result;
}

void token_template::validate()const
{
	//check not_buy_asset_handle
	FC_ASSERT(not_buy_asset_handle >= token_rule::not_buy_asset_handle_way::dispatch_to_buyer && not_buy_asset_handle <= token_rule::not_buy_asset_handle_way::burn_asset);
}

/*void token_options::validate()const
{

	FC_ASSERT(create_time<=prediction_interval);
	// vote core asset AFT 1.3.0
	FC_ASSERT(creator_vote.asset_id == asset_id_type());

}
void extend_options::validate()const
{
}
*/
void token_publish_operation::validate()const
{
	//ilog("validate ~${fee}", fee.amount);
	FC_ASSERT( fee.amount >= 0 );

	template_parameter.validate();
	//if(exts)		exts->validate();

	FC_ASSERT(template_parameter.guaranty_core_asset_amount.asset_id == asset_id_type());
}

share_type token_publish_operation::calculate_fee( const fee_parameters_type& k )const
{
   	return k.fee;
}

void token_buy_operation::validate()const
{
	FC_ASSERT( fee.amount >= 0 );
	//FC_ASSERT(template_vote.unit.base == "CNY" and template_vote.unit.quote == "BTC");


	//FC_ASSERT(template_parameter.buy_quote_amount.asset_id == asset_id_type());
	FC_ASSERT(template_parameter.buy_quantity > 0);
}

share_type token_buy_operation::calculate_fee( const fee_parameters_type& k )const
{
   	return k.fee;
}

void token_event_operation::validate()const
{
	FC_ASSERT( fee.amount >= 0 );

	FC_ASSERT(event == "create" || event == "phase1_begin" || event == "phase1_end" || event == "phase2_begin" || event == "phase2_end" ||
			event == "settle" || event == "return_asset_end" || event == "close" || event == "restore" || event == "set_control", 
			"invalid event, event=${event}", ("event", event));
}

share_type token_event_operation::calculate_fee( const fee_parameters_type& k )const
{
   	return k.fee;
}


} } // graphene::chain
