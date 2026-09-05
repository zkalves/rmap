/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
            const std::string &base_dir = ""
        );

        // Generation for all templates found in a single folder
        GenerationReport parseDirectory(
            const json &json_data,
            const std::string &template_folder,
            const std::string &output_folder,
            const std::string &base_dir = ""
        );

        // Legacy compatibility methods
        void parse(json json_data, const std::string &template_folder = PathUtils::DEFAULT_TEMPLATES_DIR, const std::string &output_folder = PathUtils::DEFAULT_OUTPUT_DIR);
        void parseCustom(json json_data, const std::string &template_folder, const std::vector<TemplateMapping>& mappings);

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
