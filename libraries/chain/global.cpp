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
#include <graphene/chain/global.hpp>
#include <graphene/chain/word_object.hpp>

namespace graphene { namespace chain {


//判断一个词是否包含敏感词, 敏感词库不使用大写字母
string word_contain_sensitive_word(string& word, graphene::chain::database& d)
{
    const auto& idx = d.get_index_type<word_index>().indices().get<by_id>();
    auto itr = idx.begin();
    if (itr != idx.end())
    {
        for(auto itr2 = itr->sensitive_words.begin(); itr2 != itr->sensitive_words.end(); ++itr2)
        {
            if ( word.find(*itr2) != string::npos )
            {
                return *itr2;
            }
        }
    }
    return "";
}


// 判断一个字符串是否包含敏感词, 敏感词库不使用大写字母
string string_contain_sensitive_word(string& s, graphene::chain::database& d)
{
    const auto& idx = d.get_index_type<word_index>().indices().get<by_id>();
    auto itr = idx.begin();
    if (itr != idx.end())
    {
        unsigned int len = s.length();
        string result;
        string word;
        char c;
        //unsigned int pos = 0;
        bool is_a_word_end = false;
        bool is_ascii = false;

        for(unsigned int i = 0; i < len; ++i)
        {
            c = s[i];

            if( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) //如果是ascii码，是英文字母
            {
                //word[pos++] = c;
                word.push_back(c);
                is_ascii = true;
            }
            else if( c < 0 ) //如果不是ascii码
            {
                //word[pos++] = c;
                word.push_back(c);
                is_ascii = false;
            }

            //切割单词

            // 如果是ascii码，但不是英文字母
            // 当前字符是ascii码，但下一个字符不是ascii码
            // 当前字符不是ascii码，但下一个字符是ascii码
            // 最后一个字符
            if ( ( c >= 0 && !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) ||
                 (is_ascii && s[i+1] < 0) || 
                 (!is_ascii && s[i+1] >= 0) || 
                 i == len - 1 )
            {
                is_a_word_end = true;
            }

            if(is_a_word_end && word != "") //处理一个词
            {
                result = word_contain_sensitive_word(word, d);
                if ( result != "" )
                {
                    return result;
                }

                //reset
                //word = "";
                word.clear();
                //pos = 0;
                is_a_word_end = false;
            }
        }
    }
    return "";
}


} } // graphene::global
