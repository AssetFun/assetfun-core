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

#include <graphene/chain/protocol/subject.hpp>

namespace graphene { namespace chain {

typedef static_variant<subject_rule::price_unit, int> subject_rule_unit;

template<typename SubjectTemplateType>
static SubjectTemplateType subject_template_check(const variant& from)
{
	SubjectTemplateType result;
	from_variant(from, result);
	return result;
}

void subject_template::validate()const
{
	FC_ASSERT(event == "price" || event == "score" || event == "temperature");
	if(event == "price") {
		//subject_rule::price_unit rule_unit = subject_template_check<subject_rule::price_unit>(unit);
		//FC_ASSERT(unit.base == "CNY" and unit.quote == "BTC");
	}

	if(event == "score") {
		//int rule_unit = subject_template_check<int>(unit);
		//FC_ASSERT(rule_unit == 1);
	}	
	// FC_ASSERT(unit.base == "CNY" and unit.quote == "BTC");

}

void subject_options::validate()const
{

	FC_ASSERT(create_time<=prediction_interval);
	// vote core asset AFT 1.3.0
	FC_ASSERT(creator_vote.asset_id == asset_id_type());

}
void extend_options::validate()const
{
}

void subject_publish_operation::validate()const
{
	//ilog("validate ~${fee}", fee.amount);
	FC_ASSERT( fee.amount >= 0 );

	if(content)		content->template_subject.validate();
	if(opts)		opts->validate();
	if(exts)		exts->validate();



}

share_type subject_publish_operation::calculate_fee( const fee_parameters_type& k )const
{
	auto core_fee_required = k.basic_fee;  
   	if( content )
      	core_fee_required += get_pred_interval() / 3600 * k.price_per_hour;
   	return core_fee_required;
}

void subject_vote_operation::validate()const
{
	FC_ASSERT( fee.amount >= 0 );
	//FC_ASSERT(template_vote.unit.base == "CNY" and template_vote.unit.quote == "BTC");

	// vote core asset AFT 1.3.0
	FC_ASSERT(template_vote.quantity.asset_id == asset_id_type());
}

share_type subject_vote_operation::calculate_fee( const fee_parameters_type& k )const
{
   	return k.fee_percent_of_vote_amount * template_vote.quantity.amount / GRAPHENE_100_PERCENT; // 0.8% of vote amount
}

void subject_event_operation::validate()const
{
	FC_ASSERT( fee.amount >= 0 );
}

share_type subject_event_operation::calculate_fee( const fee_parameters_type& k )const
{
   	return k.fee;
}


} } // graphene::chain
