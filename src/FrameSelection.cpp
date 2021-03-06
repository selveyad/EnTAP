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


//*********************** Includes *****************************
#include "FrameSelection.h"
#include "ExceptionHandler.h"
#include "EntapGlobals.h"
#include "frame_selection/ModGeneMarkST.h"
#include "FileSystem.h"
//**************************************************************


/**
 * ======================================================================
 * Function FrameSelection(std::string &input, std::string &out,
 *                             boost::program_options::variables_map &user_flags,
 *                             GraphingManager *graphingManager)
 *
 * Description           - Initializes member variables of Frame Selection
 *                       - Sets software type
 *
 * Notes                 - Called from EntapExecute
 *
 * @param input          - Input transcriptome (may have been from expression analysis)
 * @param out            - EnTAP main output directory
 * @param user_flag      - Boost map of user inputs
 * @param graphingManager- Pointer to graphing manager
 *
 * @return               - FrameSelection object
 *
 * =====================================================================
 */
FrameSelection::FrameSelection(std::string &input, EntapDataPtrs &entap_data) {
    FS_dprint("Spawn Object - FrameSelection");

    _pGraphingManager = entap_data._pGraphingManager;
    _QUERY_DATA       = entap_data._pQueryData;
    _exe_path         = GENEMARK_EXE;
    _inpath           = input;
    _pFileSystem      = entap_data._pFileSystem;
    _pUserInput       = entap_data._pUserInput;

    _entap_data_ptrs = entap_data;

    _outpath         = _pFileSystem->get_root_path();
    _overwrite       = _pUserInput->has_input(_pUserInput->INPUT_FLAG_OVERWRITE);
    _software_flag   = FRAME_GENEMARK_ST;

    _mod_out_dir   = PATHS(_outpath, FRAME_SELECTION_OUT_DIR);
}


/**
 * ======================================================================
 * Function std::string FrameSelection::execute(std::string input,
 *                                              std::map<std::string,QuerySequence> &SEQUENCES)
 *
 * Description           - Handles overall Frame Selection process
 *                       - Runs software, parses and adds information to existing
 *                         data
 *
 * Notes                 - None
 *
 * @param input          - Input transcriptome (may have been from expression analysis)
 * @param SEQUENCES      - Sequence map of information thus far
 *
 * @return               - Path to new transcriptome
 *
 * =====================================================================
 */
std::string FrameSelection::execute(std::string input) {

    std::string output;
    EntapModule::ModVerifyData verify_data;
    std::unique_ptr<AbstractFrame> ptr;


    _inpath = input;
    if (_overwrite) _pFileSystem->delete_dir(_mod_out_dir);
    _pFileSystem->create_dir(_mod_out_dir);
    try {
        ptr = spawn_object();
        verify_data = ptr->verify_files();
        if (!verify_data.files_exist) {
            ptr->execute();
            output = ptr->get_final_faa();
        } else output = verify_data.output_paths[0];
        ptr->parse();
        ptr.reset();
        return output;
    } catch (const ExceptionHandler &e) {
        ptr.reset();
        throw e;
    }
}


/**
 * ======================================================================
 * Function std::unique_ptr<AbstractFrame> FrameSelection::spawn_object()
 *
 * Description           - Spawns object for specified frame selection software
 *
 * Notes                 - Currently only GeneMarkS-T support
 *
 *
 * @return               - Frame Selection module object
 *
 * =====================================================================
 */
std::unique_ptr<AbstractFrame> FrameSelection::spawn_object() {
    // Handle any special conditions for each software

    switch (_software_flag) {
        case FRAME_GENEMARK_ST:
            return std::unique_ptr<AbstractFrame>(new ModGeneMarkST(
                    _mod_out_dir,
                    _inpath,
                    _entap_data_ptrs,
                    _exe_path
            ));
        default:
            return std::unique_ptr<AbstractFrame>(new ModGeneMarkST(
                    _mod_out_dir,
                    _inpath,
                    _entap_data_ptrs,
                    _exe_path
            ));
    }
}

