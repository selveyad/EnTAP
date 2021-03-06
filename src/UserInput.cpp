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
#include "UserInput.h"
#include "ExceptionHandler.h"
#include "GraphingManager.h"
#include "SimilaritySearch.h"
#include "common.h"
#include "ontology/ModEggnog.h"
#include "ontology/ModInterpro.h"
#include "version.h"
#include "database/EntapDatabase.h"
#include "ontology/ModEggnogDMND.h"
#include "config.h"
#include "similarity_search/ModDiamond.h"

//**************************************************************

//*********************** Defines ******************************
#define DESC_HELP           "Print all the help options for this version of EnTAP!"
#define DESC_CONFIG         "Configure EnTAP for execution later.\n"                    \
                            "If this is your first time running EnTAP run this first!"  \
                            "This will perform the following:\n"                        \
                            "    - Downloading EnTAP taxonomic database\n"              \
                            "    - Downloading Gene Ontology term database\n"           \
                            "    - Formatting any database you would like for diamond"
#define DESC_RUN_PROTEIN    "Execute EnTAP functionality through blastp\n"              \
                            "Note, if your input sequences are nucleotide, they will be"\
                            "frame selected automatically."
#define DESC_RUN_NUCLEO     "Execute EnTAP functionality through blastx\n"              \
                            "This will not frame select your sequences and will run them"\
                            "through each stage of the pipeline as nucelotide sequences"
#define DESC_INTER_DATA     "Select which databases you would like for InterProScan"    \
                            "Databases must be one of the following:\n"                 \
                            "    -tigrfam\n"                                            \
                            "    -sfld\n"                                               \
                            "    -prodom\n"                                             \
                            "    -hamap\n"                                              \
                            "    -pfam\n"                                               \
                            "    -smart\n"                                              \
                            "    -cdd\n"                                                \
                            "    -prositeprofiles\n"                                    \
                            "    -prositepatterns\n"                                    \
                            "    -superfamily\n"                                        \
                            "    -prints\n"                                             \
                            "    -panther\n"                                            \
                            "    -gene3d\n"                                             \
                            "    -pirsf\n"                                              \
                            "    -coils\n"                                              \
                            "    -morbidblite\n"                                        \
                            "Make sure the database is downloaded, EnTAP will not check!"
#define DESC_ONTOLOGY_FLAG  "Specify the ontology software you would like to use\n"     \
                            "Note: it is possible to specify more than one! Just use"   \
                            "multiple --ontology flags\n"                               \
                            "Specify flags as follows:\n"                               \
                            "    0. EggNOG (default)\n"                                 \
                            "    1. InterProScan"
#define DESC_GRAPHING       "Check whether or not your system supports graphing.\n"     \
                            "This option does not require any other flags and will"     \
                            "just check whether the version of Python being used has"   \
                            "MatPlotLib accessible."
#define DESC_OUT_FLAG       "Specify the output directory you would like the data to"   \
                            " be saved to."
#define DESC_DATABASE       "Provide the paths to the databases you would like to use\n"\
                            "For running: ensure the databases selected are .dmnd"      \
                            "formatted.\n"                                              \
                            "For configuration: ensure the databases are FASTA format\n"\
                            "Note: if your databases are not NCBI or Uniprot\n"         \
                            "databases, taxonomic filtering might not be able to pull"  \
                            "the species information!"
#define DESC_ONT_LEVELS     "Specify the Gene Ontology levels you would like printed\n" \
                            "Default: 0, 3, 4\n"                                        \
                            "A level of 0 means that every term will be printed!"       \
                            "It is possible to specify multiple flags as well with\n"   \
                            "multiple --level flags\n"                                  \
                            "Example: --level 0 --level 3 --level 1"
#define DESC_FPKM           "Specify the FPKM threshold with expression analysis\n"     \
                            "EnTAP will filter out transcripts below this value!"
#define DESC_EVAL           "Specify the E-Value that will be used as a cutoff during"  \
                            "similarity searching"
#define DESC_THREADS        "Specify the number of threads that will be used throughout\n"
#define DESC_SINGLE_END     "Specify this flag if your BAM/SAM file was generated\n"    \
                            "through single-end reads\n"                                \
                            "Note: this is only required in expression analysis\n"      \
                            "Default: paired-end"
#define DESC_ALIGN_FILE     "Specify the path to the BAM/SAM file for expression analysis"
#define DESC_CONTAMINANT    "Specify the contaminants you would like to filter out"     \
                            "from similarity searching\n"                               \
                            "Note: since hits are based upon a multitide of factors"    \
                            "a contaminant might be the best hit for a query!\n"        \
                            "Contaminants can be selected by species (homo_sapiens)"    \
                            "or through a specific taxon (homo)\n"                      \
                            "If your taxon is more than one word just replace the"      \
                            "spaces with underscores (_)"
#define DESC_NO_TRIM        "By default, EnTAP will trim the input sequences to the first space.\n"             \
                            "This helps with compatibility across different software\n"  \
                            "Example:\n"                                                \
                            ">TRINITY_231.1 Protein Information\n"                      \
                            "will become...\n"                                          \
                            ">TRINITY_231.1\n"                                          \
                            "Use this command if you would like to instead remove all\n"\
                            "spaces in your sequence headers to retain information. \n" \
                            "Warning: this may cause issues recognizing headers from your BAM or SAM files."
#define DESC_QCOVERAGE      "Select the minimum query coverage to be allowed during"    \
                            "similarity searching"
#define DESC_TCOVERAGE      "Select the minimum target coverage to be allowed during"   \
                            "similarity searching"
#define DESC_EXE_PATHS      "Specify path to the entap_config.txt file that will"       \
                            "be used to find all of the executables!"
#define DESC_DATA_GENERATE  "Specify whether you would like to generate EnTAP databases"\
                            " instead of downloading them.\nDefault: Download\nIf you"  \
                            " are encountering issues with the downloaded databases "   \
                            "you can try this"
#define DESC_DATABASE_TYPE  "Specify which EnTAP database you would like to download/"  \
                            "generate.\n"                                               \
                            "    0. Serialized Database (default)\n"                    \
                            "    1. SQLITE Database\n"                                  \
                            "Either or both can be selected with an additional flag. "  \
                            "The serialized database will be faster although requires " \
                            "more memory usage. The SQLITE database may be slightly "   \
                            "slower and does not require the Boost libraries if you "   \
                            "are experiencing any incompatibility there."
#define DESC_TAXON          "Specify the type of species/taxon you are analyzing and"   \
                            "would like hits closer in taxonomic relevance to be"       \
                            "favored (based on NCBI Taxonomic Database)\n"              \
                            "Note: formatting works just like with the contaminants"
#define DESC_STATE          "Specify the state of execution (EXPERIMENTAL)\n"           \
                            "More information is available in the documentation\n"      \
                            "This flag may have undesired affects and may not run properly!"
#define DESC_INPUT_TRAN     "Path to the input transcriptome file"
#define DESC_COMPLET_PROT   "Select this option if all of your sequences are complete"  \
                            "proteins.\n"                                               \
                            "At this point, this option will merely flag the sequences"
#define DESC_OVERWRITE      "Select this option if you would like to overwrite previous"\
                            "files\n"                                                   \
                            "Note: do NOT use this if you would like to pickup from"    \
                            "a previous run!"
#define DESC_UNINFORMATIVE  "Path to a list of keywords that should be used to specify" \
                            " uninformativeness of hits during similarity searching. "  \
                            "Generally something along the lines of 'hypothetical' or " \
                            "'unknown' are used. Each term should be on a new line of " \
                            "the file being linked to.\nExample (defaults):\n"          \
                            "    -conserved\n"                                          \
                            "    -predicted\n"                                          \
                            "    -unknown\n"                                            \
                            "    -hypothetical\n"                                       \
                            "    -putative\n"                                           \
                            "    -unidentified\n"                                       \
                            "    -uncultured\n"                                         \
                            "    -uninformative\n"                                      \
                            "Ensure each word is on a separate line in the file. EnTAP will take the " \
                            "each line as a new uninformative word!"
#define DESC_NOCHECK        "Use this flag if you don't want your input to EnTAP verifed."\
                            " This is not advised to use! Your run may fail later on "  \
                            "if inputs are not checked"
#define DESC_OUTPUT_FORMAT  "Specify the output format for the processed alignments."   \
                            "Multiple flags can be specified."                          \
                            "    1. TSV Format (default)\n"                             \
                            "    2. CSV Format\n"                                       \
                            "    3. FASTA Amino Acid (default)\n"                       \
                            "    4. FASTA Nucleotide (default)"
//**************************************************************
// Externs
std::string RSEM_EXE_DIR;
std::string GENEMARK_EXE;
std::string DIAMOND_EXE;
std::string EGG_SQL_DB_PATH;
std::string EGG_DMND_PATH;
std::string INTERPRO_EXE;
std::string ENTAP_DATABASE_BIN_PATH;
std::string ENTAP_DATABASE_SQL_PATH;
std::string GRAPHING_EXE;

//**************************************************************

/**
 * ======================================================================
 * Function void parse_arguments_boost
 *                              (int            argc,
 *                               const char**   argv)
 *
 * Description          - Utilizes boost libraries to parse user input
 *                        arguments
 *
 * Notes                - None
 *
 * @param argc          - Pushed from main
 * @param argv          - Pushed from main
 * @return              - None
 * ======================================================================
 */
#ifdef USE_BOOST
void UserInput::parse_arguments_boost(int argc, const char** argv) {
    try {
        boostPO::options_description description("Options");
        // TODO separate out into main options and additional config file with defaults
        description.add_options()
                ((INPUT_FLAG_HELP + ",h").c_str(), DESC_HELP)
                (INPUT_FLAG_CONFIG.c_str(),DESC_CONFIG)
                (INPUT_FLAG_RUNPROTEIN.c_str(),DESC_RUN_PROTEIN)
                (INPUT_FLAG_RUNNUCLEOTIDE.c_str(),DESC_RUN_NUCLEO)
                (INPUT_FLAG_UNINFORM.c_str(), boostPO::value<std::string>(),DESC_UNINFORMATIVE)
                (INPUT_FLAG_INTERPRO.c_str(),
                 boostPO::value<std::vector<std::string>>()->multitoken()
                         ->default_value(std::vector<std::string>{ModInterpro::get_default()},""),DESC_INTER_DATA)
                (INPUT_FLAG_ONTOLOGY.c_str(),
                 boostPO::value<std::vector<uint16>>()->multitoken()
                 ->default_value(std::vector<uint16>{ENTAP_EXECUTE::EGGNOG_DMND_INT_FLAG},""),DESC_ONTOLOGY_FLAG)
                (INPUT_FLAG_GRAPH.c_str(),DESC_GRAPHING)
                (INPUT_FLAG_TAG.c_str(),
                 boostPO::value<std::string>()->default_value(OUTFILE_DEFAULT),DESC_OUT_FLAG)
                ((INPUT_FLAG_DATABASE + ",d").c_str(),
                 boostPO::value<std::vector<std::string>>()->multitoken(),DESC_DATABASE)
                (INPUT_FLAG_GO_LEVELS.c_str(),
                 boostPO::value<std::vector<uint16>>()->multitoken()
                         ->default_value(std::vector<uint16>{0,3,4},""), DESC_ONT_LEVELS)
                (INPUT_FLAG_FPKM.c_str(),
                 boostPO::value<fp32>()->default_value(RSEM_FPKM_DEFAULT), DESC_FPKM)
                (INPUT_FLAG_E_VAL.c_str(),
                 boostPO::value<fp64>()->default_value(E_VALUE),DESC_EVAL)
                ((INPUT_FLAG_VERSION + ",v").c_str(), "Display EnTAP release version")
                (UserInput::INPUT_FLAG_SINGLE_END.c_str(), DESC_SINGLE_END)
                ((INPUT_FLAG_THREADS + ",t").c_str(),
                 boostPO::value<int>()->default_value(1),DESC_THREADS)
                ((INPUT_FLAG_ALIGN + ",a").c_str(), boostPO::value<std::string>(),DESC_ALIGN_FILE)
                ((INPUT_FLAG_CONTAM + ",c").c_str(),
                 boostPO::value<std::vector<std::string>>()->multitoken(),DESC_CONTAMINANT)
                (INPUT_FLAG_NO_TRIM.c_str(), DESC_NO_TRIM)
                (INPUT_FLAG_QCOVERAGE.c_str(),
                 boostPO::value<fp32>()->default_value(DEFAULT_QCOVERAGE), DESC_QCOVERAGE)
                (INPUT_FLAG_EXE_PATH.c_str(), boostPO::value<std::string>(), DESC_EXE_PATHS)
                (INPUT_FLAG_GENERATE.c_str(), DESC_DATA_GENERATE)
                (INPUT_FLAG_DATABASE_TYPE.c_str(),
                 boostPO::value<std::vector<uint16>>()->multitoken()
                        ->default_value(std::vector<uint16>{EntapDatabase::ENTAP_SERIALIZED},""), DESC_DATABASE_TYPE)
                (INPUT_FLAG_TCOVERAGE.c_str(),
                 boostPO::value<fp32>()->default_value(DEFAULT_TCOVERAGE), DESC_TCOVERAGE)
                (INPUT_FLAG_SPECIES.c_str(), boostPO::value<std::string>(),DESC_TAXON)
                (INPUT_FLAG_STATE.c_str(),
                 boostPO::value<std::string>()->default_value(DEFAULT_STATE), DESC_STATE)
                ((INPUT_FLAG_TRANSCRIPTOME + ",i").c_str(), boostPO::value<std::string>(), DESC_INPUT_TRAN)
                (INPUT_FLAG_COMPLETE.c_str(), DESC_COMPLET_PROT)
                (INPUT_FLAG_NOCHECK.c_str(), DESC_NOCHECK)
                (INPUT_FLAG_OUTPUT_FORMAT.c_str(),
                 boostPO::value<std::vector<uint16>>()->multitoken()
                        ->default_value(std::vector<uint16>{FileSystem::ENT_FILE_DELIM_TSV, FileSystem::ENT_FILE_FASTA_FAA, FileSystem::ENT_FILE_FASTA_FNN},""),DESC_OUTPUT_FORMAT)
                (INPUT_FLAG_OVERWRITE.c_str(), DESC_OVERWRITE);
        boostPO::variables_map vm;
        try {
            boostPO::store(boostPO::command_line_parser(argc,argv).options(description)
                                   .run(),vm);
            boostPO::notify(vm);

            if (vm.count(INPUT_FLAG_HELP)) {
                std::cout << description<<std::endl<<std::endl;
                throw(ExceptionHandler("",ERR_ENTAP_SUCCESS));
            }
            if (vm.count(INPUT_FLAG_VERSION)) {
                std::cout<<"EnTAP version: "<<ENTAP_VERSION_STR<<std::endl;
                throw(ExceptionHandler("",ERR_ENTAP_SUCCESS));
            }
            _user_inputs = vm;
        } catch (boost::program_options::required_option& e) {
            throw ExceptionHandler(e.what(),ERR_ENTAP_INPUT_PARSE);
        }
    }catch (boost::program_options::error& e){
        // Unknown input
        throw ExceptionHandler(e.what(),ERR_ENTAP_INPUT_PARSE);
    }
}
#else // Use TCLAP for program argument parsing
/**
 * ======================================================================
 * Function void parse_arguments_tclap
 *                              (int            argc,
 *                               const char**   argv)
 *
 * Description          - Utilizes TCLAP libraries to parse user input
 *                        arguments
 *
 * Notes                - None
 *
 * @param argc          - Pushed from main
 * @param argv          - Pushed from main
 * @return              - None
 * ======================================================================
 */

 void UserInput::parse_arguments_tclap(int argc, const char ** argv) {
    try {
        TCLAP::CmdLine cmd("EnTAP\nAlexander Hart and Dr. Jill Wegrzyn\nUniversity of Connecticut\nCopyright 2017-2019", ' ', ENTAP_VERSION_STR);

        // Switch Args
        TCLAP::SwitchArg argConfig("",INPUT_FLAG_CONFIG, DESC_CONFIG, cmd, false);
        TCLAP::SwitchArg argProtein("", INPUT_FLAG_RUNPROTEIN, DESC_RUN_PROTEIN, cmd, false);   // can xor these
        TCLAP::SwitchArg argNucleo("", INPUT_FLAG_RUNNUCLEOTIDE, DESC_RUN_NUCLEO, cmd, false);
        TCLAP::SwitchArg argGraph("", INPUT_FLAG_GRAPH, DESC_GRAPHING, cmd, false);
        TCLAP::SwitchArg argTrim("", INPUT_FLAG_NO_TRIM, DESC_NO_TRIM, cmd, false);
        TCLAP::SwitchArg argGenerate("", INPUT_FLAG_GENERATE, DESC_DATA_GENERATE, cmd, false);
        TCLAP::SwitchArg argComplete("", INPUT_FLAG_COMPLETE, DESC_COMPLET_PROT, cmd, false);
        TCLAP::SwitchArg argNoCheck("", INPUT_FLAG_NOCHECK, DESC_NOCHECK, cmd, false);
        TCLAP::SwitchArg argOverwrite("", INPUT_FLAG_OVERWRITE, DESC_OVERWRITE, cmd, false);
        TCLAP::SwitchArg argSingleEnd("", INPUT_FLAG_SINGLE_END, DESC_SINGLE_END, cmd, false);

        // Value Args
        TCLAP::ValueArg<std::string> argUninform("", INPUT_FLAG_UNINFORM, DESC_UNINFORMATIVE, false, "", "string", cmd);
        TCLAP::ValueArg<std::string> argTag("", INPUT_FLAG_TAG, DESC_OUT_FLAG, false, OUTFILE_DEFAULT, "string", cmd);
        TCLAP::ValueArg<fp32> argFPKM("", INPUT_FLAG_FPKM, DESC_FPKM, false, RSEM_FPKM_DEFAULT, "decimal", cmd);
        TCLAP::ValueArg<fp64> argEval("", INPUT_FLAG_E_VAL, DESC_EVAL, false, E_VALUE, "decimal", cmd);
        TCLAP::ValueArg<int> argThreads("t", INPUT_FLAG_THREADS, DESC_THREADS, false, DEFAULT_THREADS, "integer", cmd);
        TCLAP::ValueArg<std::string> argAlign("a", INPUT_FLAG_ALIGN, DESC_ALIGN_FILE, false, "","string",cmd);
        TCLAP::ValueArg<fp32> argQueryCov("", INPUT_FLAG_QCOVERAGE, DESC_QCOVERAGE, false, DEFAULT_QCOVERAGE, "decimal", cmd);
        TCLAP::ValueArg<std::string> argExePath("", INPUT_FLAG_EXE_PATH, DESC_EXE_PATHS, false, "", "string", cmd);
        TCLAP::ValueArg<fp32> argTCoverage("", INPUT_FLAG_TCOVERAGE, DESC_TCOVERAGE, false, DEFAULT_TCOVERAGE, "decimal", cmd);
        TCLAP::ValueArg<std::string> argSpecies("", INPUT_FLAG_SPECIES, DESC_TAXON, false, "", "string", cmd);
        TCLAP::ValueArg<std::string> argState("", INPUT_FLAG_STATE, DESC_STATE, false, DEFAULT_STATE, "string", cmd);
        TCLAP::ValueArg<std::string> argTranscript("i", INPUT_FLAG_TRANSCRIPTOME, DESC_INPUT_TRAN, false, "", "string", cmd);

        // Multi Args
        TCLAP::MultiArg<std::string> argInterpro("", INPUT_FLAG_INTERPRO, DESC_INTER_DATA, false, "string list",cmd);
        TCLAP::MultiArg<uint16> argOntology("", INPUT_FLAG_ONTOLOGY, DESC_ONTOLOGY_FLAG, false, "integer list", cmd);
        TCLAP::MultiArg<std::string> argDatabase("d", INPUT_FLAG_DATABASE, DESC_DATABASE, false, "string list", cmd);
        TCLAP::MultiArg<uint16> argGOLevels("", INPUT_FLAG_GO_LEVELS, DESC_ONT_LEVELS, false, "integer list", cmd);
        TCLAP::MultiArg<std::string> argContam("c", INPUT_FLAG_CONTAM, DESC_CONTAMINANT, false, "string list", cmd  );
        TCLAP::MultiArg<uint16> argDataType("", INPUT_FLAG_DATABASE_TYPE, DESC_DATABASE_TYPE, false, "integer list", cmd);
        TCLAP::MultiArg<uint16> argOutputFormat("", INPUT_FLAG_OUTPUT_FORMAT, DESC_OUTPUT_FORMAT, false, "integer list", cmd);

        cmd.parse( argc, argv );

        // if no inputs, print usage and exit
        if (argc == 1) {
            TCLAP::StdOutput out;
            out.usage(cmd);
            throw ExceptionHandler("",0);
        }

        /* Add everything to the map that will be used throughout execution */

        // Add SwitchArgs
        if (argConfig.isSet()) _user_inputs.emplace(INPUT_FLAG_CONFIG, true);
        if (argProtein.isSet())_user_inputs.emplace(INPUT_FLAG_RUNPROTEIN, true);
        if (argNucleo.isSet()) _user_inputs.emplace(INPUT_FLAG_RUNNUCLEOTIDE, true);
        if (argGraph.isSet()) _user_inputs.emplace(INPUT_FLAG_GRAPH, true);
        if (argTrim.isSet()) _user_inputs.emplace(INPUT_FLAG_NO_TRIM, true);
        if (argGenerate.isSet()) _user_inputs.emplace(INPUT_FLAG_GENERATE, true);
        if (argComplete.isSet()) _user_inputs.emplace(INPUT_FLAG_COMPLETE, true);
        if (argNoCheck.isSet()) _user_inputs.emplace(INPUT_FLAG_NOCHECK, true);
        if (argOverwrite.isSet()) _user_inputs.emplace(INPUT_FLAG_OVERWRITE, true);
        if (argSingleEnd.isSet()) _user_inputs.emplace(INPUT_FLAG_SINGLE_END, true);

        // Add ValueArgs
        if (argUninform.isSet())_user_inputs.emplace(INPUT_FLAG_UNINFORM, argUninform.getValue());
        _user_inputs.emplace(INPUT_FLAG_TAG, argTag.getValue());
        _user_inputs.emplace(INPUT_FLAG_FPKM, argFPKM.getValue());
        _user_inputs.emplace(INPUT_FLAG_E_VAL, argEval.getValue());
        _user_inputs.emplace(INPUT_FLAG_THREADS, argThreads.getValue());
        if (argAlign.isSet()) _user_inputs.emplace(INPUT_FLAG_ALIGN, argAlign.getValue());
        _user_inputs.emplace(INPUT_FLAG_QCOVERAGE, argQueryCov.getValue());
        if (argExePath.isSet()) _user_inputs.emplace(INPUT_FLAG_EXE_PATH, argExePath.getValue());
        _user_inputs.emplace(INPUT_FLAG_TCOVERAGE, argTCoverage.getValue());
        if (argSpecies.isSet()) _user_inputs.emplace(INPUT_FLAG_SPECIES, argSpecies.getValue());
        _user_inputs.emplace(INPUT_FLAG_STATE, argState.getValue());
        if (argTranscript.isSet())_user_inputs.emplace(INPUT_FLAG_TRANSCRIPTOME, argTranscript.getValue());

        // Add MultiArgs (defaults) Couldnt find a way to do defaults in constructor??!
        if (argInterpro.isSet()) {
            _user_inputs.emplace(INPUT_FLAG_INTERPRO, argInterpro.getValue());
        } else {
            _user_inputs.emplace(INPUT_FLAG_INTERPRO, vect_str_t{ModInterpro::get_default()});
        }
        if (argOntology.isSet()) {
            _user_inputs.emplace(INPUT_FLAG_ONTOLOGY, argOntology.getValue());
        } else {
            _user_inputs.emplace(INPUT_FLAG_ONTOLOGY, std::vector<uint16>{ONT_EGGNOG_DMND});
        }
        if (argGOLevels.isSet()) {
            _user_inputs.emplace(INPUT_FLAG_GO_LEVELS, argGOLevels.getValue());
        } else {
            _user_inputs.emplace(INPUT_FLAG_GO_LEVELS, std::vector<uint16>{0,3,4});
        }
        if (argDataType.isSet()) {
            _user_inputs.emplace(INPUT_FLAG_DATABASE_TYPE, argDataType.getValue());
        } else {
            _user_inputs.emplace(INPUT_FLAG_DATABASE_TYPE, std::vector<uint16>{EntapDatabase::ENTAP_SERIALIZED});
        }

        if (argOutputFormat.isSet()) {
            _user_inputs.emplace(INPUT_FLAG_OUTPUT_FORMAT, argOutputFormat.getValue());
        } else {
            _user_inputs.emplace(INPUT_FLAG_OUTPUT_FORMAT, vect_uint16_t{FileSystem::ENT_FILE_DELIM_TSV, FileSystem::ENT_FILE_FASTA_FNN, FileSystem::ENT_FILE_FASTA_FAA});
        }

        _user_inputs.emplace(INPUT_FLAG_DATABASE, argDatabase.getValue());
        _user_inputs.emplace(INPUT_FLAG_CONTAM, argContam.getValue());

    } catch (TCLAP::ArgException &e) {
        throw ExceptionHandler(e.what(), ERR_ENTAP_INPUT_PARSE);
    }
 }




#endif //USE_BOOST

/**
 * ======================================================================
 * Function void verify_user_input(boostPO::variables_map&       vm)
 *
 * Description          - Mangages ensuring user input is valid and
 *                        will not cause issues downstream
 *
 * Notes                - None
 *
 * @param vm            - User variable map of flags
 * @return              - None
 * =====================================================================
 */
bool UserInput::verify_user_input() {

    bool                     is_interpro;
    bool                     is_protein;
    bool                     is_nucleotide;
    bool                     is_config;
    bool                     is_run;
    std::string              species;
    std::string              input_tran_path;
    std::vector<uint16>      ont_flags;
    EntapDatabase           *pEntapDatabase = nullptr;


    // If graphing flag, check if it is allowed then EXIT
    if (has_input(UserInput::INPUT_FLAG_GRAPH)) {
        if (!_pFileSystem->file_exists(GRAPHING_EXE)) {
            std::cout<<"Graphing is NOT enabled on this system! Graphing script could not "
                    "be found at: "<<GRAPHING_EXE << std::endl;
        }
        GraphingManager gmanager = GraphingManager(GRAPHING_EXE);
        if (gmanager.is_graphing_enabled()) {
            std::cout<< "Graphing is enabled on this system!" << std::endl;
            throw ExceptionHandler("",ERR_ENTAP_SUCCESS);
        } else {
            std::cout<<"Graphing is NOT enabled on this system!,"
                    " ensure that you have python with the Matplotlib module installed."<<std::endl;
            throw ExceptionHandler("",ERR_ENTAP_SUCCESS);
        }
    }


    // ------------ Config / Run Required beyond this point ---------------- //

    is_config     = has_input(INPUT_FLAG_CONFIG);     // ignore 'config config'
    is_protein    = has_input(INPUT_FLAG_RUNPROTEIN);
    is_nucleotide = has_input(INPUT_FLAG_RUNNUCLEOTIDE);

    _is_config = is_config;

    if (is_protein && is_nucleotide) {
        throw ExceptionHandler("Cannot specify both protein and nucleotide input flags",
                               ERR_ENTAP_INPUT_PARSE);
    }
    is_run = is_protein || is_nucleotide;

    // Check config and run flags
    if (!is_config && !is_run) {
        throw(ExceptionHandler("Either config option or run option are required",
                               ERR_ENTAP_INPUT_PARSE));
    } else if (is_config && is_run) {
        throw(ExceptionHandler("Cannot specify both config and run flags",
                               ERR_ENTAP_INPUT_PARSE));
    }
    print_user_input();

    // If user wants to skip this check, EXIT
    if (has_input(UserInput::INPUT_FLAG_NOCHECK)) {
        FS_dprint("User is skipping input verification!! :(");
        return is_config;
    }

    try {

        verify_databases(is_run);

        // Handle generic flags
        if (has_input(UserInput::INPUT_FLAG_OUTPUT_FORMAT)) {
            vect_uint16_t output_formats = get_user_input<vect_uint16_t>(UserInput::INPUT_FLAG_OUTPUT_FORMAT);
            for (uint16 flag : output_formats) {
                if (flag <= FileSystem::ENT_FILE_UNUSED || flag >= FileSystem::ENT_FILE_OUTPUT_FORMAT_MAX) {
                    throw ExceptionHandler("Invalid flag for Output Format (" + std::to_string(flag) + ")",
                                           ERR_ENTAP_INPUT_PARSE);
                }
            }
        }

        // Handle EnTAP execution commands
        if (is_run) {

            // Verify EnTAP database can be generated
            FS_dprint("Verifying EnTAP database...");
            pEntapDatabase = new EntapDatabase(_pFileSystem);
            // Find database type that will be used by the rest (use 0 index no matter what)
            vect_uint16_t entap_database_types =
                    get_user_input<vect_uint16_t>(INPUT_FLAG_DATABASE_TYPE);
            EntapDatabase::DATABASE_TYPE type =
                    static_cast<EntapDatabase::DATABASE_TYPE>(entap_database_types[0]);
            if (!pEntapDatabase->set_database(type)) {
                throw ExceptionHandler("Unable to open EnTAP database from paths given" + pEntapDatabase->print_error_log(),
                                       ERR_ENTAP_READ_ENTAP_DATA_GENERIC);
            }
            // Verify database type
            if (!pEntapDatabase->is_valid_version()) {
                throw ExceptionHandler("EnTAP database version invalid with this version of software\nYou have: " +
                                               pEntapDatabase->get_current_version_str() + "\nYou need: " +
                                               pEntapDatabase->get_required_version_str(), ERR_ENTAP_READ_ENTAP_DATA_GENERIC);
            }

            FS_dprint("Success!");

            // Verify input transcriptome
            if (!has_input(UserInput::INPUT_FLAG_TRANSCRIPTOME)) {
                throw(ExceptionHandler("Must enter a valid transcriptome",ERR_ENTAP_INPUT_PARSE));
            } else {
                input_tran_path = get_user_input<std::string>(INPUT_FLAG_TRANSCRIPTOME);
                if (!_pFileSystem->file_exists(input_tran_path)) {
                    throw(ExceptionHandler("Transcriptome not found at: " + input_tran_path,
                                           ERR_ENTAP_INPUT_PARSE));
                } else if (_pFileSystem->file_empty(input_tran_path)) {
                    throw(ExceptionHandler("Transcriptome file empty: "+ input_tran_path,
                                           ERR_ENTAP_INPUT_PARSE));
                } else if (!_pFileSystem->check_fasta(input_tran_path)) {
                    throw(ExceptionHandler("File not in fasta format or corrupt! "+ input_tran_path,
                                           ERR_ENTAP_INPUT_PARSE));
                }
            }

            // Verify species for taxonomic relevance
            if (has_input(INPUT_FLAG_SPECIES)) {
                verify_species(SPECIES, pEntapDatabase);
            }

            // Verify contaminant
            if (has_input(INPUT_FLAG_CONTAM)) {
                verify_species(CONTAMINANT, pEntapDatabase);
            }

            // Verify path + extension for alignment file
            if (has_input(INPUT_FLAG_ALIGN)) {
                std::string align_file = get_user_input<std::string>(INPUT_FLAG_ALIGN);
                std::string align_ext = _pFileSystem->get_file_extension(align_file, false);
                std::transform(align_ext.begin(), align_ext.end(), align_ext.begin(), ::tolower);
                if (align_ext != FileSystem::EXT_SAM && align_ext != FileSystem::EXT_BAM) {
                    throw ExceptionHandler("Alignment file must have a .bam or .sam extension",
                                           ERR_ENTAP_INPUT_PARSE);
                }
                if (!_pFileSystem->file_exists(align_file)) {
                    throw ExceptionHandler("BAM/SAM file not found at: " + align_file + " exiting...",
                                           ERR_ENTAP_INPUT_PARSE);
                }
            }

            // Verify FPKM
            if (has_input(UserInput::INPUT_FLAG_FPKM)) {
                fp32 fpkm = get_user_input<fp32>(INPUT_FLAG_FPKM);
                if (fpkm > FPKM_MAX || fpkm < FPKM_MIN) {
                    throw ExceptionHandler("FPKM is out of range, but be between " + std::to_string(FPKM_MIN) +
                                           " and " + std::to_string(FPKM_MAX), ERR_ENTAP_INPUT_PARSE);
                }
            }

            // Verify query coverage
            if (has_input(INPUT_FLAG_QCOVERAGE)) {
                fp32 qcoverage = get_user_input<fp32>(UserInput::INPUT_FLAG_QCOVERAGE);
                if (qcoverage > COVERAGE_MAX || qcoverage < COVERAGE_MIN) {
                    throw ExceptionHandler("Query coverage is out of range, but be between " +
                                           std::to_string(COVERAGE_MIN) +
                                           " and " + std::to_string(COVERAGE_MAX), ERR_ENTAP_INPUT_PARSE);
                }
            }

            // Verify target coverage
            if (has_input(INPUT_FLAG_TCOVERAGE)) {
                fp32 qcoverage = get_user_input<fp32>(INPUT_FLAG_TCOVERAGE);
                if (qcoverage > COVERAGE_MAX || qcoverage < COVERAGE_MIN) {
                    throw ExceptionHandler("Target coverage is out of range, but be between " +
                                           std::to_string(COVERAGE_MIN) +
                                           " and " + std::to_string(COVERAGE_MAX), ERR_ENTAP_INPUT_PARSE);
                }
            }

            // Verify Ontology Flags
            is_interpro = false;
            if (has_input(INPUT_FLAG_ONTOLOGY)) {
                ont_flags = get_user_input<std::vector<uint16>>(INPUT_FLAG_ONTOLOGY);
                for (uint8 i = 0; i < ont_flags.size() ; i++) {
                    if ((ont_flags[i] > ONT_SOFTWARE_COUNT) ||
                         ont_flags[i] < 0) {
                        throw ExceptionHandler("Invalid ontology flags being used",
                                               ERR_ENTAP_INPUT_PARSE);
                    }
                    if (ont_flags[i] == ONT_INTERPRO_SCAN && !is_interpro) is_interpro = true;
                }
            }

            // Verify InterPro databases
            if (is_interpro && !ModInterpro::valid_input(this)) {
                throw ExceptionHandler("InterPro selected, but invalid databases input!", ERR_ENTAP_INPUT_PARSE);
            }

            // Verify uninformative file list
            if (has_input(INPUT_FLAG_UNINFORM)) {
                std::string uninform_path = get_user_input<std::string>(UserInput::INPUT_FLAG_UNINFORM);
                verify_uninformative(uninform_path);
            }

            // Verify paths from state
            if (get_user_input<std::string>(INPUT_FLAG_STATE) == DEFAULT_STATE) {
                std::string state = DEFAULT_STATE;
                // only handling default now
                verify_state(state, is_protein, ont_flags);
            }
        } else {
            // Must be config

            // Check if EggNOG DIAMOND database exists, if not, check DIAMOND run
            if (!_pFileSystem->file_exists(EGG_DMND_PATH)) {
                if (!ModDiamond::is_executable(DIAMOND_EXE)) {
                    throw ExceptionHandler("EggNOG DIAMOND database was not found at: " + EGG_DMND_PATH +
                                           "\nThe DIAMOND test run failed.", ERR_ENTAP_INPUT_PARSE);
                }
            }

            // Test run DIAMOND if user input databases
            if (has_input(INPUT_FLAG_DATABASE)) {
                if (!ModDiamond::is_executable(DIAMOND_EXE)) {
                    throw ExceptionHandler("Databases have been selected for indexing. A test run of DIAMOND has failed!",
                                           ERR_ENTAP_INPUT_PARSE);
                }
            }
        }

    }catch (const ExceptionHandler &e) {
        delete pEntapDatabase;
        throw e;
    }
    FS_dprint("Success! Input verified");
    delete pEntapDatabase;
    return is_config;
}


/**
 * ======================================================================
 * Function void verify_databases(boostPO::variables_map& vm)
 *
 * Description          - Ensures the user is entering valid databases
 *                        and flags
 *
 * Notes                - Not really currently used, will be updated
 *
 * @param exe           - Boost variable map of user input
 * @return              - None
 * ======================================================================
 */
void UserInput::verify_databases(bool isrun) {

    databases_t     other_data;

    if (has_input(INPUT_FLAG_DATABASE)) {
        other_data = get_user_input<databases_t>(INPUT_FLAG_DATABASE);
    } else if (isrun){
        // Must specify database when executing
        throw ExceptionHandler("Must select databases when executing main pipeline", ERR_ENTAP_INPUT_PARSE);
    }

    if (other_data.size() > MAX_DATABASE_SIZE) {
        throw ExceptionHandler("Too many databases selected, the max is " + std::to_string(MAX_DATABASE_SIZE), ERR_ENTAP_INPUT_PARSE);
    }

    // Check each database entered
    for (auto const& path: other_data) {
        if (!_pFileSystem->file_exists(path) || _pFileSystem->file_empty(path)) {
            throw ExceptionHandler("Database path invalid or empty: " + path, ERR_ENTAP_INPUT_PARSE);
        }
        FS_dprint("User has input a database at: " + path);
        // Is file extension diamond?
        if (_pFileSystem->get_file_extension(path, false).compare(FileSystem::EXT_DMND) == 0) {
            // Yes, are we configuring?
            if (!isrun) {
                throw ExceptionHandler("Cannot input DIAMOND (.dmnd) database when configuring!", ERR_ENTAP_INPUT_PARSE);
            }
        } else {
            // No, are we executing main pipeline?
            if (isrun) {
                throw ExceptionHandler("Must input DIAMOND (.dmnd) database when executing!", ERR_ENTAP_INPUT_PARSE);
            }
        }

    }
}


/**
 * ======================================================================
 * Function std::unordered_map<std::string,std::string> parse_config(std::string &exe)
 *
 * Description          - Manages parsing and verifying the EnTAP configuration
 *                        file provided by the user
 *                      - Calls verification of each key to ensure user
 *                        has not changed keys
 *                      - Generates configuration file if one is not found
 *
 * Notes                - Entry
 *
 * @param exe_paths     - First: path to config file, Second: current dir
 * @return              - Map of keys to user parameters
 * ======================================================================
 */
std::unordered_map<std::string,std::string> UserInput::parse_config(pair_str_t &exe_paths) {
    FS_dprint("Parsing configuration file...");

    std::unordered_map<std::string,std::string> config_map;
    std::string                                 new_config;
    std::string                                 line;
    std::string                                 key;
    std::string                                 val;

    if (!_pFileSystem->file_exists(exe_paths.first)){
        FS_dprint("Config file not found, generating new file...");
        new_config = CONFIG_FILE;
        try {
            generate_config(new_config);
        } catch (std::exception &e){
            throw ExceptionHandler(e.what(),ERR_ENTAP_CONFIG_CREATE);
        }
        FS_dprint("Config file successfully created");
        throw ExceptionHandler("Configuration file generated at: " + exe_paths.first,
            ERR_ENTAP_CONFIG_CREATE_SUCCESS);
    }
    FS_dprint("Config file found at: " + exe_paths.first);
    std::ifstream in_file(exe_paths.first);
    while (std::getline(in_file,line)) {
        std::istringstream in_line(line);
        if (std::getline(in_line,key,'=')) {
            if (!check_key(key)) {
                throw ExceptionHandler("Incorrect format in config file at line: " + in_line.str(),
                                       ERR_ENTAP_CONFIG_PARSE);
            }
            if (std::getline(in_line,val)) {
                if (val.size()<=1) val = "";
                config_map.emplace(key,val);
            }
        }
    }
    FS_dprint("Success!");
    init_exe_paths(config_map,exe_paths.second);
    return config_map;
}


/**
 * ======================================================================
 * Function void generate_config(std::string        &path)
 *
 * Description          - Generates configuration file if it is not found
 *                        previously
 *
 * Notes                - Called from function parse_config()
 *
 * @param path          - Path to where the file should be written
 * @return              - None
 * =====================================================================
 */
void UserInput::generate_config(std::string &path) {
    std::ofstream config_file(path, std::ios::out | std::ios::app);
    config_file <<
                KEY_DIAMOND_EXE               <<"=\n"<<
                KEY_RSEM_EXE                  <<"=\n"<<
                KEY_GENEMARK_EXE              <<"=\n"<<
                KEY_EGGNOG_SQL_DB             <<"=\n"<<
                KEY_EGGNOG_DMND               <<"=\n"<<
                KEY_INTERPRO_EXE              <<"=\n"<<
                KEY_ENTAP_DATABASE_SQL        <<"=\n"<<
                KEY_ENTAP_DATABASE_BIN        <<"=\n"<<
                KEY_GRAPH_SCRIPT              <<"=\n"
                << std::endl;
    config_file.close();
}


/**
 * ======================================================================
 * Function bool check_key(std::string&     key)
 *
 * Description          - Ensures EnTAP configuration file has valid
 *                        entries and has not been edited by user
 *
 * Notes                - Called from function parse_config()
 *
 * @param key           - Key from configuration file
 * @return              - Flag if key is valid or not
 * =====================================================================
 */
bool UserInput::check_key(std::string& key) {
    LOWERCASE(key);
    if (key == KEY_DIAMOND_EXE)      return true;
    if (key == KEY_GENEMARK_EXE)     return true;
    if (key == KEY_EGGNOG_SQL_DB)        return true;
    if (key == KEY_EGGNOG_DMND)        return true;
    if (key == KEY_INTERPRO_EXE)     return true;
    if (key == KEY_ENTAP_DATABASE_BIN) return true;
    if (key == KEY_ENTAP_DATABASE_SQL) return true;
    if (key == KEY_GRAPH_SCRIPT)     return true;
    return key == KEY_RSEM_EXE;
}


/**
 * ======================================================================
 * Function void print_user_input()
 *
 * Description          - Handles printing of user selected flags to
 *                        EnTAP statistics/log file
 *
 * Notes                - Called from main
 *
 * @return              - None
 *
 * =====================================================================
 */
void UserInput::print_user_input() {

    std::string         output;
    std::stringstream   ss;
    std::string         config_text;
    std::time_t         time;
    std::chrono::time_point<std::chrono::system_clock> start_time;

    start_time = std::chrono::system_clock::now();
    time = std::chrono::system_clock::to_time_t(start_time);
    _is_config ? config_text = "Configuration" : config_text = "Execution";

    _pFileSystem->format_stat_stream(ss, "EnTAP Run Information - " + config_text);

    ss <<
       "Current EnTAP Version: "   << ENTAP_VERSION_STR            <<
       "\nStart time: "            << std::ctime(&time)            <<
       "\nWorking directory has been set to: "  << _pFileSystem->get_root_path()<<
       "\n\nExecution Paths/Commands:"
       "\n\nRSEM Directory: "                  << RSEM_EXE_DIR      <<
       "\nGeneMarkS-T: "                       << GENEMARK_EXE      <<
       "\nDIAMOND: "                           << DIAMOND_EXE       <<
       "\nInterPro: "                          << INTERPRO_EXE      <<
       "\nEggNOG SQL Database: "               << EGG_SQL_DB_PATH   <<
       "\nEggNOG DIAMOND Database: "           << EGG_DMND_PATH     <<
       "\nEnTAP Database (binary): "           << ENTAP_DATABASE_BIN_PATH <<
       "\nEnTAP Database (SQL): "              << ENTAP_DATABASE_SQL_PATH <<
       "\nEnTAP Graphing Script: "             << GRAPHING_EXE      <<
       "\n\nUser Inputs:\n";

    // Print all user inputs (fairly raw right now)

    for (const auto& it : _user_inputs) {
        std::string key = it.first.c_str();
        ss << "\n" << key << ": ";
#ifdef USE_BOOST
        auto& value = it.second.value();
#else
        auto& value = it.second;
#endif
        if (auto v = boost::any_cast<std::string>(&value)) {
            ss << *v;
        } else if (auto v = boost::any_cast<std::vector<std::string>>(&value)) {
            if (v->size()>0) {
                for (auto const& val:*v) {
                    ss << val << " ";
                }
            } else ss << "null";
        } else if (auto v = boost::any_cast<float>(&value)){
            ss << *v;
        } else if (auto v = boost::any_cast<double>(&value)) {
            ss << float_to_sci(*v,2);
        } else if (auto v = boost::any_cast<int>(&value)) {
            ss << *v;
        } else if (auto v = boost::any_cast<std::vector<short>>(&value)) {
            for (auto const &val:*v) {
                ss << val << " ";
            }
        } else if (auto v = boost::any_cast<vect_uint16_t>(&value)) {
            for (auto const &val:*v) {
                ss << val << " ";
            }
        } else ss << "null";
    }
    output = ss.str() + "\n";
    _pFileSystem->print_stats(output);
    FS_dprint(output+"\n");
}


/**
 * ======================================================================
 * Function void verify_species(boostPO::variables_map &map)
 *
 * Description          - Verify species/tax level input by the user
 *                      - Ensure it can be found within the tax database
 *                      - Ensure it's in the right format
 *
 * Notes                - None
 *
 * @param exe           - Boost map of user inputs
 * @return              - None
 * ======================================================================
 */
void UserInput::verify_species(SPECIES_FLAGS flag, EntapDatabase *database) {

    std::vector<std::string> species;
    std::string              raw_species;

    if (database == nullptr) return;

    if (flag == SPECIES) {
        raw_species = get_target_species_str();
        species.push_back(raw_species);
    } else if (flag == CONTAMINANT) {
        species = get_user_input<std::vector<std::string>>(INPUT_FLAG_CONTAM);
        for (std::string &contam : species) {
            process_user_species(contam);
        }
    }
    if (species.empty()) return;

    for (std::string &s : species) {
        if (database->get_tax_entry(s).is_empty()) {
            throw ExceptionHandler("Error in one of your inputted taxons: " + s + " it is not located"
                                   " within the taxonomic database. You may remove it or select another",
                                    ERR_ENTAP_INPUT_PARSE);
        }
    }
    FS_dprint("Taxonomic species verified");
}



/**
 * ======================================================================
 * Description      - Finds exe paths for each piece of pipeline. WARNING: does not
 *                    check for validity of exe paths (as some parts may not want to be
 *                    ran)
 *
 * @param map       - Map of entap_config file
 * @param exe       - EnTAP execution directory
 * @return          - DIAMOND .exe path ran by enTAP::Init
 * ======================================================================
 */
void UserInput::init_exe_paths(std::unordered_map<std::string, std::string> &map, std::string exe) {
    FS_dprint("Assigning execution paths. Note they are not checked for validity yet...");

    std::stringstream                  ss;
    std::string                        out_msg;
    std::pair<std::string,std::string> outpair;
    std::string temp_rsem              = map[KEY_RSEM_EXE];
    std::string temp_diamond           = map[KEY_DIAMOND_EXE];
    std::string temp_genemark          = map[KEY_GENEMARK_EXE];
    std::string temp_interpro          = map[KEY_INTERPRO_EXE];
    std::string temp_eggnog_sql_db     = map[KEY_EGGNOG_SQL_DB];
    std::string temp_eggnog_dmnd_db    = map[KEY_EGGNOG_DMND];
    std::string temp_entap_sql_db      = map[KEY_ENTAP_DATABASE_SQL];
    std::string temp_entap_bin_db      = map[KEY_ENTAP_DATABASE_BIN];
    std::string temp_graphing          = map[KEY_GRAPH_SCRIPT];

    // Included software paths
    if (temp_rsem.empty())    temp_rsem           = PATHS(exe,Defaults::RSEM_DEFAULT_EXE);
    if (temp_diamond.empty()) temp_diamond        = PATHS(exe,Defaults::DIAMOND_DEFAULT_EXE);
    if (temp_genemark.empty())temp_genemark       = PATHS(exe,Defaults::GENEMARK_DEFAULT_EXE);
    if (temp_interpro.empty())   temp_interpro    = Defaults::INTERPRO_DEF_EXE;

    // EggNOG paths
    if (temp_eggnog_sql_db.empty())  temp_eggnog_sql_db   = PATHS(exe,Defaults::EGG_SQL_DB_DEFAULT);
    if (temp_eggnog_dmnd_db.empty())  temp_eggnog_dmnd_db   = PATHS(exe,Defaults::EGG_DMND_DEFAULT);

    // EnTAP paths
    if (temp_entap_sql_db.empty()) temp_entap_sql_db = PATHS(exe, Defaults::ENTAP_DATABASE_SQL_DEFAULT);
    if (temp_entap_bin_db.empty()) temp_entap_bin_db = PATHS(exe, Defaults::ENTAP_DATABASE_BIN_DEFAULT);
    if (temp_graphing.empty()) temp_graphing = PATHS(exe, Defaults::GRAPH_SCRIPT_DEF);

    DIAMOND_EXE      = temp_diamond;
    GENEMARK_EXE     = temp_genemark;
    RSEM_EXE_DIR     = temp_rsem;
    EGG_SQL_DB_PATH  = temp_eggnog_sql_db;
    EGG_DMND_PATH    = temp_eggnog_dmnd_db;
    INTERPRO_EXE     = temp_interpro;
    ENTAP_DATABASE_BIN_PATH = temp_entap_bin_db;
    ENTAP_DATABASE_SQL_PATH = temp_entap_sql_db;
    GRAPHING_EXE     = temp_graphing;

    FS_dprint("Success! All exe paths set");
}


/**
 * ======================================================================
 * Function std::string get_config_path()
 *
 * Description          - Gets the configuration file path either from
 *                        user input or default
 *                      - Calls separate routine to pull execution directory
 *                        for defaults
 *
 *
 * @return              - Pair (first:config path, second:exe directory)
 * ======================================================================
 */
pair_str_t UserInput::get_config_path() {
    //TODO check different systems

    pair_str_t output;      // first:config file, second:default for exe paths

    if (has_input(INPUT_FLAG_EXE_PATH)) {
        output.first = get_user_input<std::string>(INPUT_FLAG_EXE_PATH);
        FS_dprint("User input config filepath at: " + output.first);
    } else {
        // if no config input, use default path found in cwd
        output.first = PATHS(_pFileSystem->get_cur_dir(), CONFIG_FILE);
        FS_dprint("No inputted config file, using default: " + output.first);
    }
    if (!_pFileSystem->file_exists(output.first)) {
        FS_dprint("No configuration file with execution paths found at: " +
                output.first);
        // Will be printed later
    }

    output.second = get_executable_dir();
    return output;
}


/**
 * ======================================================================
 * Function void process_user_species(std::string &input)
 *
 * Description          - Format species user has input
 *
 * Notes                - Throw error on failure
 *
 * @param input         - Species to be formatted
 * @return              - None
 * ======================================================================
 */
void UserInput::process_user_species(std::string &input) {
    LOWERCASE(input);
    STR_REPLACE(input, '_', ' ');
}


/**
 * ======================================================================
 * Function void verify_uninformative(std::string& path)
 *
 * Description          - Sanity check on uninformative list from user
 *                      - Only checks existance/read
 *
 * Notes                - Throw error on failure
 *
 * @param path          - Path to user file
 * @return              - None
 * ======================================================================
 */
void UserInput::verify_uninformative(std::string& path) {
    if (!_pFileSystem->file_exists(path) || _pFileSystem->file_empty(path) ||
            !_pFileSystem->file_test_open(path)) {
        throw ExceptionHandler("Path to uninformative list invalid/empty!",ERR_ENTAP_INPUT_PARSE);
    }
}


/**
 * ======================================================================
 * Function void verify_state(std::string &state, bool runP,
 *                            std::vector<uint16> &ontology)
 *
 * Description          - Entry to check execution paths for software based
 *                        on state
 *
 * Notes                - Throw error on failure
 *
 * @param state         - State inputted by user (or default)
 * @param runP          - Blastp flag (yes/no)
 * @param ontology      - Vector of ontology flags
 *
 * @return              - None
 * ======================================================================
 */
void UserInput::verify_state(std::string &state, bool runP, std::vector<uint16> &ontology) {
    uint8 execute = 0x0;
    std::pair<bool, std::string> out;
    if (state.compare(DEFAULT_STATE) == 0) {
        execute |= SIMILARITY_SEARCH;
        execute |= GENE_ONTOLOGY;
    }
    out = verify_software(execute, ontology);
    if (!out.first) throw ExceptionHandler(out.second, ERR_ENTAP_INPUT_PARSE);
}


/**
 * ======================================================================
 * Function std::pair<bool,std::string> verify_software(uint8 &states,
 *                                      std::vector<uint16> &ontology)
 *
 * Description          - Sanity check on software that will be used during
 *                        execution
 *
 * Notes                - None
 *
 * @param states        - State flags
 * @param ontology      - Vector of ontology flags
 *
 * @return              - Pair of yes/no failure and error msg string
 * ======================================================================
 */
std::pair<bool,std::string> UserInput::verify_software(uint8 &states,std::vector<uint16> &ontology) {
    FS_dprint("Verifying software...");

    if (states & SIMILARITY_SEARCH) {
        if (!ModDiamond::is_executable(DIAMOND_EXE)) {
            return std::make_pair(false, "Could not execute a test run of DIAMOND, be sure"
                    " it's properly installed and the path is correct");
        }
    }
    if (states & GENE_ONTOLOGY) {
        for (uint16 flag : ontology) {
            switch (flag) {
#ifdef EGGNOG_MAPPER
                case ENTAP_EXECUTE::EGGNOG_INT_FLAG:
                    if (!_pFileSystem->file_exists(EGG_SQL_DB_PATH))
                        return std::make_pair(false, "Could not find EggNOG SQL database at: " + EGG_SQL_DB_PATH);
                    if (!ModEggnog::is_executable())
                        return std::make_pair(false, "Test of EggNOG Emapper failed, "
                                "ensure python is properly installed and the paths are correct");
                    break;
#endif
                case ONT_INTERPRO_SCAN:
                    // TODO
                    break;

                case ONT_EGGNOG_DMND:
                    if (!_pFileSystem->file_exists(EGG_SQL_DB_PATH))
                        return std::make_pair(false, "Could not find EggNOG SQL database at: " + EGG_SQL_DB_PATH);
                    else if (!_pFileSystem->file_exists(EGG_DMND_PATH))
                        return std::make_pair(false, "Could not find EggNOG Diamond Database at: " + EGG_DMND_PATH);
                    else if (!ModEggnogDMND::is_executable(DIAMOND_EXE))
                        return std::make_pair(false, "Test run of DIAMOND for EggNOG analysis has failed");
                default:
                    break;
            }
        }
    }

    FS_dprint("Success!");
    return std::make_pair(true, "");
}

UserInput::UserInput(int argc, const char** argv) {
    FS_dprint("Spawn Object - UserInput");

#ifdef USE_BOOST
    parse_arguments_boost(argc, argv);
#else
    parse_arguments_tclap(argc, argv);
#endif
}

UserInput::~UserInput() {
    FS_dprint("Killing object - UserInput");
}

void UserInput::set_pFileSystem(FileSystem *_pFileSystem) {
    UserInput::_pFileSystem = _pFileSystem;
}

bool UserInput::has_input(const std::string &key) {
#ifdef USE_BOOST
    return (bool)_user_inputs.count(key);
#else
    return _user_inputs.find(key) != _user_inputs.end();
#endif
}


/**
 * ======================================================================
 * Function int get_supported_threads(boost::program_options::variables_map &user_map)
 *
 * Description          - Gets threads supported by system and compared with
 *                        user selection
 *                      - If thread support is lower, set to that
 *
 * Notes                - None
 *
 * @param user_map      - Boost parsed input
 *
 * @return              - thread number
 *
 * =====================================================================
 */
int UserInput::get_supported_threads() {

    uint32       supported_threads;
    int          threads;
    int          user_threads;

    supported_threads = std::thread::hardware_concurrency();
    user_threads = get_user_input<int>(INPUT_FLAG_THREADS);
    // assuming positive
    if ((uint32) user_threads > supported_threads) {
        FS_dprint("Specified thread number is larger than available threads,"
                                        "setting threads to " + std::to_string(supported_threads));
        threads = supported_threads;
    } else {
        threads = user_threads;
    }
    return threads;
}

std::queue<char> UserInput::get_state_queue() {
    std::string state_str;
    std::queue<char> out_queue;

    if (has_input(INPUT_FLAG_STATE)) {
        state_str = get_user_input<std::string>(UserInput::INPUT_FLAG_STATE);
        for (char c : state_str) {
            out_queue.push(c);
        }
    }
    return out_queue;   // check on return end if empty
}

std::string UserInput::get_target_species_str() {
    std::string input_species;

    if (has_input(INPUT_FLAG_SPECIES)) {
        input_species = get_user_input<std::string>(UserInput::INPUT_FLAG_SPECIES);
        process_user_species(input_species);
        return input_species;
    } else return "";
}

vect_str_t UserInput::get_contaminants() {
    vect_str_t output_contams;

    if (has_input(INPUT_FLAG_CONTAM)) {
        output_contams = get_user_input<vect_str_t>(UserInput::INPUT_FLAG_CONTAM);
        for (std::string &contam : output_contams) {
            if (contam.empty()) continue;
            process_user_species(contam);
        }
    }
    return output_contams;
}

vect_str_t UserInput::get_uninformative_vect() {
    vect_str_t output_uninform;
    std::string uninform_path;

    if (has_input(INPUT_FLAG_UNINFORM)) {
        uninform_path = get_user_input<std::string>(UserInput::INPUT_FLAG_UNINFORM);
    } else {
        return INFORMATIVENESS;
    }

    std::ifstream file(uninform_path);
    std::string line;

    while (getline(file,line)) {
        if (line.empty()) continue;
        LOWERCASE(line);
        line.erase(std::remove(line.begin(), line.end(), '\n'),line.end());
        output_uninform.push_back(line);
    }
    file.close();
    return output_uninform;
}

std::string UserInput::get_user_transc_basename() {
    std::string user_transcriptome;

    user_transcriptome = get_user_input<std::string>(INPUT_FLAG_TRANSCRIPTOME);
    return _pFileSystem->get_filename(user_transcriptome, false);
}


/**
 * ======================================================================
 * Function std::string UserInput::get_executable_dir()
 *
 * Description          - Gets the directory that the executable was detected
 *                        in
 *
 * Notes                - Only implemented for UNIX systems now!
 *
 *
 * @return              - Path to directory that EnTAP exe was found
 * ======================================================================
 */
std::string UserInput::get_executable_dir() {
    char buff[1024];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
        buff[len] = '\0';
        std::string path = std::string(buff);
        return path.substr(0,path.find_last_of('/'));
    }
    FS_dprint("EnTAP execution path was NOT found!");
    return "";
}

std::vector<FileSystem::ENT_FILE_TYPES> UserInput::get_user_output_types() {
    std::vector<FileSystem::ENT_FILE_TYPES> ret;

    for (uint16 val :get_user_input<std::vector<uint16>>(INPUT_FLAG_OUTPUT_FORMAT)) {
        ret.push_back(static_cast<FileSystem::ENT_FILE_TYPES>(val));
    }
    return ret;
}

