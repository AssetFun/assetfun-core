


#include <graphene/chain/exceptions.hpp>
//#include <graphene/chain/protocol/types.hpp>
//#include <graphene/chain/protocol/subject.hpp>
#include <graphene/chain/subject_object.hpp>

#include <iostream>
#define BOOST_TEST_MODULE AssetFun Subject Test
#include <boost/test/included/unit_test.hpp>

#include "../common/database_fixture.hpp"
using namespace graphene::chain;
using namespace graphene::chain::test;


struct opt_s {

	string a;
	string b;
	string c;
	string d;
	string e;

};

BOOST_FIXTURE_TEST_SUITE( subject_tests, database_fixture )

BOOST_AUTO_TEST_CASE(subject_basic_test)
{

	try {
		BOOST_TEST_MESSAGE("Subject Basic Test");

		auto nathan_private_key 	= generate_private_key("nathan");
		account_id_type nathan_id 	= create_account("nathan", nathan_private_key.get_public_key()).id;
		transfer(account_id_type(), nathan_id, asset(10000000));

		auto vote_private_key 	= generate_private_key("vote");
		account_id_type vote_id 	= create_account("vote", vote_private_key.get_public_key()).id;

		subject_publish_operation 	op;
		op.creator		= nathan_id;
		op.subject_name	= "subject-test";
		op.fee			= asset(78);

		fc::time_point_sec now 		= fc::time_point_sec(GRAPHENE_TESTING_GENESIS_TIMESTAMP);
		fc::time_point_sec after 	= fc::time_point_sec(GRAPHENE_TESTING_GENESIS_TIMESTAMP) + fc::hours(10);

		subject_options options = {now, after, asset(10)};
		op.opts 		= options;

		subject_rule::price_unit unit;
        unit.platform_id = "1000001";
        unit.quote_base  = "BTC/CNY";
		//fc::variant unit_v(unit);

		std::map<string, subject_rule::option_value> mapOpts;
		subject_rule::option_value value = {"2000", "3000"};
		subject_rule::option_value value1 = {"3000", "3000"};
		mapOpts.insert(std::make_pair("0", value));
		mapOpts.insert(std::make_pair("1", value1));
		//opt_s opt = {"2000", "3000"};
		//fc::variant options_v(opt);

		subject_template template_ = {"price", unit, after, "a", "mutil_radio", mapOpts};
		subject_content content = {"test description", template_};
		op.content 		= content;

		trx.operations.push_back(op);

		sign( trx, nathan_private_key );
		trx.validate();
   		processed_transaction ptx = db.push_transaction( trx );
   		const subject_object& subject = db.get<subject_object>(ptx.operation_results[0].get<object_id_type>());
		trx.clear();
   		ptx.operation_results.clear();

   		BOOST_TEST_MESSAGE("Subject Basic Test ~ vote");
   		subject_vote_operation vop;
   		vop.voter 		= vote_id;
   		vop.subject_id 	= subject.id;
   		vop.template_vote = {asset(0), "b", "b"};
   		trx.operations.push_back(vop);
   		sign( trx, vote_private_key );
   		ptx = db.push_transaction( trx );
   		const subject_vote_object& vote = db.get<subject_vote_object>(ptx.operation_results[0].get<object_id_type>());
		trx.clear();
		ptx.operation_results.clear();
		BOOST_CHECK(vote.subject_id == subject.id);

   		BOOST_TEST_MESSAGE("Subject Basic Test ~ event");
   		upgrade_to_lifetime_member(nathan_id);

   		subject_event_operation eop;
   		eop.oper 		= nathan_id;
   		eop.subject_id	= subject.id;
   		eop.event 		= "vote_end";
   		eop.options 	= {};
   		trx.operations.push_back(eop);
   		sign( trx, nathan_private_key );
   		//ptx = db.push_transaction( trx );
   		const subject_event_object& event = db.get<subject_event_object>(ptx.operation_results[0].get<object_id_type>());
		trx.clear();
		ptx.operation_results.clear();

		BOOST_CHECK(event.subject_id == subject.id);
		
   		
		


	} catch (fc::exception& e) {
		edump((e.to_detail_string()));
		throw;
	}

}

//BOOST_AUTO_TEST_CASE(subject_vote_test)
//{

//}

BOOST_AUTO_TEST_SUITE_END()

FC_REFLECT( opt_s, (a)(b)(c)(d)(e) )
