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


//*********************** Includes *****************************
#include <iomanip>
#include "../common.h"
#include "ModGeneMarkST.h"
#include "../ExceptionHandler.h"
#include "../EntapExecute.h"
//**************************************************************


/**
 * ======================================================================
 * Function std::pair<bool, std::string> ModGeneMarkST::verify_files()
 *
 * Description           - Checks whether GeneMarkS-T has already been ran
 *                         with the same input
 *
 * Notes                 - None
 *
 *
 * @return               - Bool   - Yes if previous files have been found
 *                       - String - Path to frame selected transcriptome
 *
 * =====================================================================
 */
std::pair<bool, std::string> ModGeneMarkST::verify_files() {
    std::string lst_file;

    print_debug("Beggining to verify GeneMark module files...");

    boost::filesystem::path file_name(boostFS::path(_inpath).filename());
    _final_out = PATHS(_frame_outpath, file_name) + ".faa";
    lst_file   = file_name.string() + ".lst";
    _final_lst = PATHS(_frame_outpath, lst_file);
    if (file_exists(_final_out) && file_exists(_final_lst)) {
        print_debug("File found at: " + _final_out + "\n"
                "continuing EnTAP with this file and skipping frame selection");
        return std::make_pair(true, _final_out);
    }
    print_debug("File not found at " + _final_out + " so continuing frame selection");
    return std::make_pair(false, "");
}


/**
 * ======================================================================
 * Function std::string ModGeneMarkST::execute(std::map<std::string, QuerySequence> &SEQUENCES)
 *
 * Description          - Calculates final statistical information after
 *                        completed execution
 *                      - Compiles stats on each stage of pipeline
 *
 * Notes                - None
 *
 * @param SEQUENCES     - Map of each query sequence + data
 *
 * @return              - None
 *
 * =====================================================================
 */
std::string ModGeneMarkST::execute() {
    // Outfiles: file/path.faa, file/path.fnn
    // assumes working directory as output right now
    std::string     lst_file;
    std::string     out_gmst_log;
    std::string     out_hmm_file;
    std::string     genemark_cmd;
    std::string     genemark_std_out;
    std::string     line;
    std::string     temp_name;
    std::string     out_path;

    boost::filesystem::path file_name(boostFS::path(_inpath).filename());
    std::list<std::string> out_names {file_name.string()+".faa",
                                      file_name.string()+".fnn"};
    lst_file     = file_name.string() + ".lst";
    out_gmst_log = PATHS(_frame_outpath, GENEMARK_LOG_FILE);
    out_hmm_file = PATHS(_frame_outpath, GENEMARK_HMM_FILE);

    genemark_cmd     = _exe_path + " -faa -fnn " + _inpath;
    genemark_std_out = PATHS(_frame_outpath, GENEMARK_STD_OUT);
    print_debug("Running genemark...\n" + genemark_cmd);

    if (execute_cmd(genemark_cmd,genemark_std_out) != 0 ) {
        throw ExceptionHandler("Error in running genemark at file located at: " +
                               _inpath, ENTAP_ERR::E_INIT_INDX_DATA_NOT_FOUND);
    }
    print_debug("Success!");
    // Format genemarks-t output (remove blank lines)
    print_debug("Formatting genemark files...");

    for (std::string path : out_names) {
        std::ifstream in_file(path);
        temp_name = path + FILE_ALT_EXT;
        out_path  = PATHS(_frame_outpath, path);
        std::ofstream out_file(path + FILE_ALT_EXT);
        while (getline(in_file,line)){
            if (!line.empty()) {
                out_file << line << '\n';
            }
        }
        in_file.close();out_file.close();
        if (remove(path.c_str())!=0 || rename(temp_name.c_str(),out_path.c_str())!=0) {
            throw ExceptionHandler("Error formatting/moving genemark results", ENTAP_ERR::E_INIT_TAX_READ);
        }
    }
    if (rename(lst_file.c_str(),_final_lst.c_str())!=0 ||
        rename(GENEMARK_LOG_FILE.c_str(),out_gmst_log.c_str())!=0 ) {
        throw ExceptionHandler("Error moving genemark results", ENTAP_ERR::E_INIT_TAX_READ);
    }
    if (file_exists(GENEMARK_HMM_FILE)) {
        rename(GENEMARK_HMM_FILE.c_str(),out_hmm_file.c_str());
    }
    print_debug("Success!");
    return _final_out;
}


/**
 * ======================================================================
 * Function void ModGeneMarkST::parse()
 *
 * Description          - Parses/calculates statistics of GeneMark output
 *
 * Notes                - None
 *
 *
 * @return              - None
 *
 * =====================================================================
 */
void ModGeneMarkST::parse() {
    // generate maps, query->sequence
    print_debug("Beginning to calculate Genemark statistics...");

    std::string                             out_removed_path;
    std::string                             out_internal_path;
    std::string                             out_complete_path;
    std::string                             out_partial_path;
    std::string                             figure_results_path;
    std::string                             figure_results_png;
    std::string                             figure_removed_path;
    std::string                             figure_removed_png;
    std::string                             min_removed_seq;
    std::string                             min_kept_seq;
    std::string                             max_removed_seq;
    std::string                             max_kept_seq;
    std::stringstream                       stat_output;
    std::map<std::string, std::ofstream*>   file_map;
    std::map<std::string, uint64>           count_map;
    std::vector<uint16>                     all_kept_lengths;
    std::vector<uint16>                     all_lost_lengths;
    uint16                                  length;
    fp32                                    avg_selected;
    fp32                                    avg_lost;
    std::pair<uint64, uint64>               kept_n;
    GraphingStruct                          graphingStruct;

    boostFS::remove_all(_processed_path);
    boostFS::remove_all(_figure_path);
    boostFS::create_directories(_processed_path);
    boostFS::create_directories(_figure_path);
    out_removed_path    = PATHS(_processed_path, FRAME_SELECTION_LOST);
    out_internal_path   = PATHS(_processed_path, FRAME_SELECTION_INTERNAL);
    out_complete_path   = PATHS(_processed_path, FRAME_SELECTION_COMPLTE);
    out_partial_path    = PATHS(_processed_path, FRAME_SELECTION_PARTIAL);
    figure_removed_path = PATHS(_figure_path, GRAPH_TEXT_REF_COMPAR);
    figure_removed_png  = PATHS(_figure_path, GRAPH_FILE_REF_COMPAR);
    figure_results_path = PATHS(_figure_path, GRAPH_TEXT_FRAME_RESUTS);
    figure_results_png  = PATHS(_figure_path, GRAPH_FILE_FRAME_RESUTS);

    // all nucleotide lengths
    uint32 min_removed=10000;
    uint32 min_selected=10000;
    uint32 max_removed=0;
    uint32 max_selected=0;
    uint64 total_removed_len=0;
    uint64 total_kept_len=0;
    uint64 count_selected=0;
    uint64 count_removed=0;
    uint64 count_partial_5=0;
    uint64 count_partial_3=0;
    uint64 count_internal=0;
    uint64 count_complete=0;

    try {
        std::map<std::string,frame_seq> protein_map = genemark_parse_protein(_final_out);
        genemark_parse_lst(_final_lst,protein_map);

        std::ofstream file_figure_removed(figure_removed_path,std::ios::out | std::ios::app);
        std::ofstream file_figure_results(figure_results_path,std::ios::out | std::ios::app);

        file_figure_removed << "flag\tsequence length" << std::endl;    // First line placeholder, not used
        file_figure_results << "flag\tsequence length" << std::endl;

        file_map[FRAME_SELECTION_LOST_FLAG] =
                new std::ofstream(out_removed_path, std::ios::out | std::ios::app);
        file_map[FRAME_SELECTION_INTERNAL_FLAG] =
                new std::ofstream(out_internal_path, std::ios::out | std::ios::app);
        file_map[FRAME_SELECTION_COMPLETE_FLAG] =
                new std::ofstream(out_complete_path, std::ios::out | std::ios::app);
        file_map[FRAME_SELECTION_FIVE_FLAG] =
                new std::ofstream(out_partial_path, std::ios::out | std::ios::app);
        file_map[FRAME_SELECTION_THREE_FLAG] = file_map[FRAME_SELECTION_FIVE_FLAG];

        count_map ={
                {FRAME_SELECTION_INTERNAL_FLAG,count_internal},
                {FRAME_SELECTION_COMPLETE_FLAG,count_complete},
                {FRAME_SELECTION_FIVE_FLAG    ,count_partial_5},
                {FRAME_SELECTION_THREE_FLAG   ,count_partial_3},
        };

        for (auto& pair : *pQUERY_DATA->get_sequences_ptr()) {
            std::map<std::string,frame_seq>::iterator p_it = protein_map.find(pair.first);
            if (!pair.second.is_is_expression_kept()) continue; // Skip seqs that were lost to expression
            if (p_it != protein_map.end()) {
                // Kept sequence, either partial, complete, or internal
                count_selected++;
                pair.second.setSequence(p_it->second.sequence);
                pair.second.setFrame(p_it->second.frame_type);

                std::string sequence = p_it->second.sequence;
                std::string frame_type = p_it->second.frame_type;
                length = (uint16) pair.second.getSeq_length();  // Nucleotide sequence length

                if (length < min_selected) {
                    min_selected = length;
                    min_kept_seq = pair.first;
                }
                if (length > max_selected) {
                    max_selected = length;
                    max_kept_seq = pair.first;
                }
                total_kept_len += length;
                all_kept_lengths.push_back(length);
                file_figure_removed << GRAPH_KEPT_FLAG << '\t' << std::to_string(length) << std::endl;
                std::map<std::string, std::ofstream*>::iterator file_it = file_map.find(frame_type);
                if (file_it != file_map.end()) {
                    *file_it->second << sequence;
                    count_map[frame_type]++;
                } else {
                    throw ExceptionHandler("Unknown frame flag found", ENTAP_ERR::E_RUN_GENEMARK_STATS);
                }

            } else {
                // Lost sequence
                count_removed++;
                pair.second.set_kept(false);
                *file_map[FRAME_SELECTION_LOST_FLAG] << pair.second.get_sequence_n() << std::endl;
                length = (uint16) pair.second.getSeq_length();  // Nucleotide sequence length

                if (length < min_removed) {
                    min_removed = length;
                    min_removed_seq = pair.first;
                }
                if (length > max_removed) {
                    max_removed_seq = pair.first;
                    max_removed = length;
                }
                file_figure_removed << GRAPH_REJECTED_FLAG << '\t' << std::to_string(length) << std::endl;
                all_lost_lengths.push_back(length);
                total_removed_len += length;
            }
        }
        for(auto& pair : file_map) {
            if (pair.first.compare(FRAME_SELECTION_THREE_FLAG)!=0){
                pair.second->close();
                delete pair.second;
                pair.second = 0;
            }
        }
        // Calculate and print stats
        print_debug("Beginning to calculate statistics...");
        avg_selected = (fp32)total_kept_len / count_selected;
        avg_lost     = (fp32)total_removed_len / count_removed;
        stat_output<<std::fixed<<std::setprecision(2);
        stat_output <<
                    ENTAP_STATS::SOFTWARE_BREAK             <<
                    "Frame Selection: GenemarkS-T\n"        <<
                    ENTAP_STATS::SOFTWARE_BREAK             <<
                    "Total sequences frame selected: "      << count_selected          <<
                    "\n\tTranslated protein sequences: "    << _final_out              <<
                    "\nTotal sequences removed (no frame): "<< count_removed           <<
                    "\n\tFrame selected CDS removed: "      << out_removed_path        <<
                    "\nTotal of "                           <<
                    count_map[FRAME_SELECTION_FIVE_FLAG]    << " 5 prime partials and "<<
                    count_map[FRAME_SELECTION_THREE_FLAG]   << " 3 prime partials"     <<
                    "\n\tPartial CDS: "                     << out_partial_path        <<
                    "\nTotal of "                           <<
                    count_map[FRAME_SELECTION_COMPLETE_FLAG]<<" complete genes:\n\t" << out_complete_path<<
                    "\nTotal of "                           <<
                    count_map[FRAME_SELECTION_INTERNAL_FLAG]<<" internal genes:\n\t" << out_internal_path<<"\n\n";

        stat_output <<
                    ENTAP_STATS::SOFTWARE_BREAK                                <<
                    "Frame Selection: New Reference Transcriptome Statistics\n"<<
                    ENTAP_STATS::SOFTWARE_BREAK;

        kept_n = pQUERY_DATA->calculate_N_vals(all_kept_lengths,total_kept_len);
        stat_output <<
                    "\n\tTotal sequences: "      << count_selected <<
                    "\n\tTotal length of transcriptome(bp): "      << total_kept_len <<
                    "\n\tAverage length(bp): "   << avg_selected   <<
                    "\n\tn50: "                  << kept_n.first   <<
                    "\n\tn90: "                  << kept_n.second  <<
                    "\n\tLongest sequence(bp): " << max_selected   << " (" << max_kept_seq << ")" <<
                    "\n\tShortest sequence(bp): "<< min_selected   << " (" << min_kept_seq << ")";

        if (count_removed > 0) {
            std::pair<uint64, uint64> removed_n =
                    pQUERY_DATA->calculate_N_vals(all_lost_lengths,total_removed_len);
            stat_output <<
                        "\nRemoved Sequences (no frame):"           <<
                        "\n\tTotal sequences: "                     << count_removed    <<
                        "\n\tAverage sequence length(bp): "         << avg_lost         <<
                        "\n\tn50: "                                 << removed_n.first  <<
                        "\n\tn90: "                                 << removed_n.second <<
                        "\n\tLongest sequence(bp): "  << max_removed<< " (" << max_removed_seq << ")" <<
                        "\n\tShortest sequence(bp): " << min_removed<< " (" << min_removed_seq << ")" <<"\n";
        }
        std::string stat_out_msg = stat_output.str();
        print_statistics(stat_out_msg);
        print_debug("Success!");

        //------------------- Figure handling ---------------//
        print_debug("Beggining figure handling...");
        file_figure_results << GRAPH_REJECTED_FLAG           << '\t' << std::to_string(count_removed)   <<std::endl;
        file_figure_results << FRAME_SELECTION_FIVE_FLAG     << '\t' << std::to_string(count_map[FRAME_SELECTION_FIVE_FLAG]) <<std::endl;
        file_figure_results << FRAME_SELECTION_THREE_FLAG    << '\t' << std::to_string(count_map[FRAME_SELECTION_THREE_FLAG]) <<std::endl;
        file_figure_results << FRAME_SELECTION_COMPLETE_FLAG << '\t' << std::to_string(count_map[FRAME_SELECTION_COMPLETE_FLAG])   <<std::endl;
        file_figure_results << FRAME_SELECTION_INTERNAL_FLAG << '\t' << std::to_string(count_map[FRAME_SELECTION_INTERNAL_FLAG])   <<std::endl;

        graphingStruct.text_file_path = figure_results_path;
        graphingStruct.graph_title    = GRAPH_TITLE_FRAME_RESULTS;
        graphingStruct.fig_out_path   = figure_results_png;
        graphingStruct.software_flag  = GRAPH_FRAME_FLAG;
        graphingStruct.graph_type     = GRAPH_PIE_RESULTS_FLAG;
        pGraphingManager->graph(graphingStruct);

        graphingStruct.text_file_path = figure_removed_path;
        graphingStruct.graph_title    = GRAPH_TITLE_REF_COMPAR;
        graphingStruct.fig_out_path   = figure_removed_png;
        graphingStruct.graph_type     = GRAPH_COMP_BOX_FLAG;
        pGraphingManager->graph(graphingStruct);
        print_debug("Success!");
        //---------------------------------------------------//

        FrameStats final_stats;
        final_stats.removed   = count_removed;
        final_stats.selected  = count_selected;
        final_stats.partial_3 = count_partial_3;
        final_stats.partial_5 = count_partial_5;
        final_stats.internal  = count_internal;
        final_stats.complete  = count_complete;
        pQUERY_DATA->set_frame_stats(final_stats);

    } catch (ExceptionHandler &e) {throw e;}
    print_debug("Success! Parsing complete");
}


/**
 * ======================================================================
 * Function ModGeneMarkST::frame_map_t ModGeneMarkST::genemark_parse_protein
 *                                                  (std::string &protein)
 *
 * Description          - Analyzes protein file to pull protein sequences
 *                      - Will be added to overall data flow
 *
 * Notes                - None
 *
 * @param protein       - Path to protein (.faa) file
 *
 * @return              - Structure containing protein sequence with seqID
 *
 * =====================================================================
 */
ModGeneMarkST::frame_map_t ModGeneMarkST::genemark_parse_protein(std::string &protein) {
    print_debug("Parsing protein file at: " + protein);

    frame_map_t     protein_map;
    frame_seq       protein_sequence;
    std::string     line;
    std::string     sequence;
    std::string     seq_id;
    std::string     sub;
    uint16          first;
    uint16          second;
    uint16          seq_len;
    uint16          line_chars;

    if (!file_exists(protein)) {
        throw ExceptionHandler("File located at: " + protein + " does not exist!",
                               ENTAP_ERR::E_RUN_GENEMARK_PARSE);
    }
    std::ifstream in_file(protein);
    while(true) {
        getline(in_file,line);
        if (line.empty() && !in_file.eof()) continue;
        if (line.find(">")==0 || in_file.eof()) {
            if (!seq_id.empty()) {
                if (in_file.eof()) {
                    sequence += line + "\n";
                }
                sub        = sequence.substr(sequence.find("\n")+1);
                line_chars = (uint16) std::count(sub.begin(),sub.end(),'\n');
                seq_len    = (uint16) (sub.length() - line_chars);
                protein_sequence = {seq_len,sequence, ""};
                protein_map.emplace(seq_id,protein_sequence);
            }
            if (in_file.eof()) break;
            first    = (uint16) (line.find(">")+1);
            second   = (uint16) line.find("\t");
            seq_id   = line.substr(first,second-first);
            sequence = ">" + seq_id + "\n";
        } else {
            sequence += line + "\n";
        }
    }

    print_debug("Success!");
    in_file.close();
    return protein_map;
}


/**
 * ======================================================================
 * Function void ModGeneMarkST::genemark_parse_lst(std::string &lst_path,
 *                                                 frame_map_t &current_map)
 *
 * Description          - Parses .lst file produced from GeneMark run
 *                      - Will add this information to map
 *
 * Notes                - None
 *
 * @param lst_path     - Path to .lst file produced from GeneMark
 * @param current_map  - Map being used to analyze GeneMark data
 *
 * @return              - None
 *
 * =====================================================================
 */
void ModGeneMarkST::genemark_parse_lst(std::string &lst_path, frame_map_t &current_map) {
    print_debug("Parsing file at: " + lst_path);

    std::string     frame;
    std::string     line;
    std::string     seq_id;
    bool            prime_5;
    bool            prime_3;
    uint16          first;

    std::ifstream in_file(lst_path);
    while (getline(in_file,line)) {
        if (line.empty()) continue;
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        if (line.find("FASTA") == 0) {
            first = (uint16) (line.find(":") + 1);
            seq_id = line.substr(first);
        } else if (isdigit(line.at(0))) {
            prime_5 = line.find("<") != std::string::npos;
            prime_3 = line.find(">") != std::string::npos;
            if (prime_5 && prime_3) {
                frame = FRAME_SELECTION_INTERNAL_FLAG;
            } else if (!prime_5 && !prime_3) {
                frame = FRAME_SELECTION_COMPLETE_FLAG;
            } else if (prime_5) {
                frame = FRAME_SELECTION_FIVE_FLAG;
            } else frame = FRAME_SELECTION_THREE_FLAG;
            std::map<std::string, frame_seq>::iterator it = current_map.find(seq_id);
            if (it != current_map.end()) {
                it->second.frame_type = frame;
            } else {
                throw ExceptionHandler("Sequence: " + seq_id + " not found in map",
                                       ENTAP_ERR::E_RUN_GENEMARK_PARSE);
            }
        }
    }
    print_debug("Success!");
}