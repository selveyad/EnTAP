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


#ifndef ENTAP_MODGENEMARKST_H
#define ENTAP_MODGENEMARKST_H


#include "AbstractFrame.h"

class ModGeneMarkST : public AbstractFrame{

struct frame_seq {
    unsigned long length;
    std::string sequence;
    std::string frame_type;
};
typedef std::map<std::string,ModGeneMarkST::frame_seq> frame_map_t;

public:
    ModGeneMarkST(std::string &execution_stage_path, std::string &in_hits,
                  EntapDataPtrs &entap_data, std::string &exe);

    ~ModGeneMarkST();

    virtual ModVerifyData verify_files() override ;
    virtual void execute() override ;
    virtual void parse() override ;

    virtual std::string get_final_faa() override ;

private:

    const std::string GRAPH_TITLE_FRAME_RESULTS     = "Frame_Selection_ORFs";
    const std::string GRAPH_FILE_FRAME_RESUTS       = "frame_results_pie.png";
    const std::string GRAPH_TEXT_FRAME_RESUTS       = "frame_results_pie.txt";
    const std::string GRAPH_TITLE_REF_COMPAR        = "Frame_Selected_Sequences";
    const std::string GRAPH_FILE_REF_COMPAR         = "removed_comparison_box.png";
    const std::string GRAPH_TEXT_REF_COMPAR         = "removed_comparison_box.txt";
    const std::string GRAPH_REJECTED_FLAG           = "Removed";
    const std::string GRAPH_KEPT_FLAG               = "Selected";

    const uint8       GRAPH_FRAME_FLAG              = 1;    // Ensure these match with entap_graphing.py
    const uint8       GRAPH_PIE_RESULTS_FLAG        = 1;
    const uint8       GRAPH_COMP_BOX_FLAG           = 2;

    const std::string GENEMARK_LOG_FILE             = "gms.log";
    const std::string GENEMARK_HMM_FILE             = "GeneMark_hmm.mod";
    const std::string GENEMARK_STD_OUT              = "genemark_run";
    const std::string FRAME_SELECTION_PARTIAL       = "partial_genes";
    const std::string FRAME_SELECTION_COMPLTE       = "complete_genes";
    const std::string FRAME_SELECTION_INTERNAL      = "internal_genes";
    const std::string FRAME_SELECTION_LOST          = "sequences_removed";
    const std::string FRAME_SELECTION_LOST_FLAG     = "lost";
    const std::string FRAME_SELECTION_FIVE_FLAG     = "Partial 5 Prime";
    const std::string FRAME_SELECTION_THREE_FLAG    = "Partial 3 Prime";
    const std::string FRAME_SELECTION_COMPLETE_FLAG = "Complete";
    const std::string FRAME_SELECTION_INTERNAL_FLAG = "Internal";

    std::string _final_faa_path;
    std::string _final_fnn_path;
    std::string _final_lst_path;
    std::string _final_gmst_log_path;
    std::string _final_hmm_path;
    std::string _transcriptome_filename;    // Filename of input transcriptome


    frame_map_t genemark_parse_fasta(std::string& path);
    void genemark_parse_lst(std::string &, std::map<std::string, frame_seq>&);
};


#endif //ENTAP_MODGENEMARKST_H
