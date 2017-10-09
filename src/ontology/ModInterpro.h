/*
 *
 * Developed by Alexander Hart
 * Plant Computational Genomics Lab
 * University of Connecticut
 *
 * For information, contact Alexander Hart at:
 *     entap.dev@gmail.com
 *
 * Copyright 2017, Alexander Hart, Dr. Jill Wegrzyn
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


#ifndef ENTAP_MODINTERPRO_H
#define ENTAP_MODINTERPRO_H


#include "AbstractOntology.h"

#define XML_ATR(x)      "<xmlattr>.x"

class ModInterpro : public AbstractOntology{

    struct InterProData {
        std::string interID;
        std::string interDesc;
        std::string databaseID;
        std::string databasetype;
        std::string databaseDesc;
        std::string pathways;
        std::string go_terms;
        fp64        eval;
    };

public:
    ModInterpro(std::string &exe,std::string &out, std::string &in,
        std::string &in_no_hits,std::string &ont, GraphingManager *graphing,QueryData *queryData,
        bool blastp,std::vector<uint16>& lvls, uint8 threads) :
    AbstractOntology(exe, out, in, in_no_hits, ont, graphing, queryData, blastp,
                     lvls, threads){}


    virtual std::pair<bool, std::string> verify_files() override ;
    virtual void execute() override ;
    virtual void parse() override ;
    virtual void set_data(std::string&, std::vector<std::string>&) override ;


private:
    std::string INTERPRO_OUTPUT             = "interpro_results";
    std::string INTERPRO_DIRECTORY          = "InterProScan";
    std::string INTERPRO_STD_OUT            = "interproscan_std";
    std::string INTERPRO_EXT                = ".xml";
    static constexpr short INTERPRO_COL_NUM = 15;

    std::string XML_SIGNATURE = "signature";
    std::string XML_ENTRY     = "entry";
    std::string XML_XREF      = "xref";
    std::string XML_PRO_M     = "protein-matches";
    std::string XML_PROTEIN   = "protein";
    std::string XML_MATCHES   = "matches";

    std::string FLAG_GOTERM   = " --goterms";
    std::string FLAG_IPRLOOK  = " --iprlookup";
    std::string FLAG_PATHWAY  = " --pathways";

    std::vector<std::string> _databases;
    std::string              _interpro_dir;
    std::string              _proc_dir;
    std::string              _figure_dir;
    std::string              _final_outpath;
    std::string              _final_basepath;
};


#endif //ENTAP_MODINTERPRO_H
