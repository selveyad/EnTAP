/*
 *
 * Developed by Alexander Hart
 * Plant Computational Genomics Lab
 * University of Connecticut
 *
 * For information, contact Alexander Hart at:
 *     entap.dev@gmail.com
 *
 * Copyright 2017-2019, Alexander Hart, Dr. Jill Wegrzyn
 *
 * This file is part of EnTAP.
 *
 * EnTAP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EnTAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EnTAP.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ENTAP_QUERYALIGNMENT_H
#define ENTAP_QUERYALIGNMENT_H

#include "QuerySequence.h"
//**********************************************************************
//**********************************************************************
//                 QueryAlignment Nested Class
//**********************************************************************
//**********************************************************************

class QueryAlignment {

public:
    QueryAlignment();
    std::string print_delim(std::vector<ENTAP_HEADERS> &, uint8 lvl, char delim);
    bool operator<(const QueryAlignment&query) {return !(*this > query);};
    void set_compare_overall_alignment(bool val);
    virtual ~QueryAlignment() = default;;
    virtual bool operator>(const QueryAlignment&)=0;
    void get_all_header_data(std::string[]);
    void get_header_data(ENTAP_HEADERS header, std::string &val, uint8 lvl);

protected:
    virtual bool is_go_header(ENTAP_HEADERS header, std::vector<std::string>& go_list)=0;

    std::unordered_map<ENTAP_HEADERS , std::string*> ALIGN_OUTPUT_MAP;
    bool _compare_overall_alignment; // May want to compare separate parameters for overall alignment across databases
    QuerySequence* _parent;
};

//**********************************************************************
//**********************************************************************
//                 SimSearchAlignment Nested Class
//**********************************************************************
//**********************************************************************

class SimSearchAlignment : public QueryAlignment{

public:
    SimSearchAlignment(QuerySequence::SimSearchResults, std::string&, QuerySequence*);
    ~SimSearchAlignment() override = default;
    QuerySequence::SimSearchResults* get_results();
    bool operator>(const QueryAlignment&) override;

private:
    void set_tax_score(std::string&);

    QuerySequence::SimSearchResults    _sim_search_results;

protected:
    bool is_go_header(ENTAP_HEADERS header, std::vector<std::string>& go_list) override;

    static constexpr uint8 E_VAL_DIF     = 8;
    static constexpr uint8 COV_DIF       = 5;
    static constexpr uint8 INFORM_ADD    = 3;
    static constexpr fp32 INFORM_FACTOR  = 1.2;
};

//**********************************************************************
//**********************************************************************
//                 EggnogDmndAlignment Nested Class
//**********************************************************************
//**********************************************************************

class EggnogDmndAlignment : public QueryAlignment {

public:
    EggnogDmndAlignment(QuerySequence::EggnogResults eggnogResults, QuerySequence* parent);
    ~EggnogDmndAlignment() override = default;
    QuerySequence::EggnogResults* get_results();
    bool operator>(const QueryAlignment&) override;
    void refresh_headers();

private:
    QuerySequence::EggnogResults _eggnog_results;

protected:
    bool is_go_header(ENTAP_HEADERS header, std::vector<std::string>& go_list) override;

};

//**********************************************************************
//**********************************************************************
//                 InterproAlignment Nested Class
//**********************************************************************
//**********************************************************************

class InterproAlignment : public QueryAlignment {

public:
    InterproAlignment(QuerySequence::InterProResults results, QuerySequence *parent);
    ~InterproAlignment() override = default;
    QuerySequence::InterProResults* get_results();
    bool operator>(const QueryAlignment&) override;


private:
    QuerySequence::InterProResults _interpro_results;

protected:
    bool is_go_header(ENTAP_HEADERS header, std::vector<std::string>& go_list) override;

};


#endif //ENTAP_QUERYALIGNMENT_H
