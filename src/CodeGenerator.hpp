/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <string>
#include <vector>
#include <utility>
#include <inja.hpp>
#include <nlohmann/json.hpp>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QString>

#include "PathUtils.hpp"

using namespace inja;
using json = nlohmann::json;

struct TemplateMapping {
    std::string template_file;
    std::string output_file;
};

struct GenerationReport {
    std::vector<std::string> success_files;
    std::vector<std::pair<std::string, std::string>> errors; // <source_path, error_message>

    bool has_errors() const { return !errors.empty(); }
};

class CodeGenerator
{
    public:
        CodeGenerator() {}
        ~CodeGenerator() {}

        // Main generation entry point supporting templates from different locations
        // and individual output dump destinations
        GenerationReport generate(
            const json &json_data,
            const std::string &default_template_folder,
            const std::string &default_output_folder,
            const std::vector<TemplateMapping> &mappings,
            const std::string &base_dir = "",
            const std::string &python_script = ""
        );

        // Generation for all templates found in a single folder
        GenerationReport parseDirectory(
            const json &json_data,
            const std::string &template_folder,
            const std::string &output_folder,
            const std::string &base_dir = "",
            const std::string &python_script = ""
        );

        // Standalone Python script execution with injected register map context variables
        bool runPythonScript(
            const std::string &python_script,
            const json &json_data,
            const std::string &base_dir = "",
            std::string *stdout_str = nullptr,
            std::string *stderr_str = nullptr
        );

        // Legacy compatibility methods
        void parse(json json_data, const std::string &template_folder = PathUtils::DEFAULT_TEMPLATES_DIR, const std::string &output_folder = PathUtils::DEFAULT_OUTPUT_DIR);
        void parseCustom(json json_data, const std::string &template_folder, const std::vector<TemplateMapping>& mappings);

        // Feature aggregation helper extracting implemented architectural attributes
        static json extractFeatures(const json &rootJson);

    private:
        void registerHelpers(Environment &env);
        std::string resolveTemplatePath(const std::string &tmpl_path, const std::string &default_folder, const std::string &base_dir = "");
        std::string resolveOutputPath(
            const std::string &tmpl_resolved_path,
            const std::string &out_path,
            const std::string &default_out_folder,
            const std::string &base_dir = "",
            const json &context = json()
        );
};

#endif // CODEGENERATOR_HPP
