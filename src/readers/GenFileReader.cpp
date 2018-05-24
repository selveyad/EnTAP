/*
 * Developed by Alexander Hart
 * Plant Computational Genomics Lab
 * University of Connecticut
 *
 * For information, contact Alexander Hart at:
 *     entap.dev@gmail.com
 *
 * Copyright 2017-2018, Alexander Hart, Dr. Jill Wegrzyn
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

#include "GenFileReader.h"

GenFileReader::GenFileReader(std::string &file_path, FileSystem::ENT_FILE_TYPES file_type,
                            FileSystem *filesystem) {

    _pFileSystem = filesystem;
    _file_type = file_type;
    _file_path = file_path;
}

bool GenFileReader::open_file() {

    if (!_pFileSystem->file_exists(_file_path) || _pFileSystem->file_empty(_file_path)) {
        return false;
    }

    switch (_file_type) {
        case FileSystem::FILE_DELIMINATED:
            break;
        default:
            break;

    }
    return false;
}
