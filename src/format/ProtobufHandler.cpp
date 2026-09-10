/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "ProtobufHandler.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/timestamp.pb.h>
#include "../SerializationContext.hpp"
#include "../ProtobufLogCollector.hpp"
#include "../RegMapTreeModel.hpp"
#include "../RegConfigWindow.hpp"

// Protobuf stream operators defined in RegMapWindow or standalone
protormap::RegModel& operator <<( protormap::RegModel& reg_model, const SerializationContext& context );
protormap::RegModel& operator >>( protormap::RegModel& reg_model, SerializationContext& context );

QString ProtobufHandler::formatName() const { return QStringLiteral("Protobuf"); }
QStringList ProtobufHandler::supportedExtensions() const { return {QStringLiteral("rmt"), QStringLiteral("rmb")}; }
QString ProtobufHandler::fileFilter() const {
    return QStringLiteral("Protobuf Register Map (*.rmt *.rmb);;Text Format (*.rmt);;Binary Format (*.rmb)");
}

FormatResult ProtobufHandler::read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    int fd = open(filepath.toLocal8Bit().constData(), O_RDONLY);
    if (fd < 0) {
        result.success = false;
        result.errorMessage = QString("Cannot open file %1: %2").arg(filepath, strerror(errno));
        return result;
    }

    protormap::RegModel reg_model;
    ProtobufLogCollector error_collector;
    google::protobuf::TextFormat::Parser parser;
    parser.RecordErrorsTo(&error_collector);

    bool parse_ok = false;
    if (filepath.endsWith(".rmb", Qt::CaseInsensitive)) {
        parse_ok = reg_model.ParseFromFileDescriptor(fd);
        ::close(fd);
    } else {
        google::protobuf::io::FileInputStream fileInput(fd);
        fileInput.SetCloseOnDelete(true);
        parse_ok = parser.Parse(&fileInput, &reg_model);
    }

    if (!parse_ok) {
        result.success = false;
        result.errorMessage = QString("Failed to parse Protobuf file %1: %2")
            .arg(filepath, QString::fromStdString(error_collector.get_string()));
        return result;
    }

    if (config && reg_model.has_config()) {
        config->deserialize(reg_model.config());
    }

    if (model && reg_model.item_size() > 0) {
        SerializationContext context;
        reg_model >> context;
        RegMapTreeItem *rootItem = context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
        if (rootItem) {
            model->setRootItem(rootItem);
        }
    }

    result.success = true;
    return result;
}

FormatResult ProtobufHandler::write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    int fd = open(filepath.toLocal8Bit().constData(), O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd < 0) {
        result.success = false;
        result.errorMessage = QString("Cannot create file %1: %2").arg(filepath, strerror(errno));
        return result;
    }

    protormap::RegModel reg_model;
    if (config) {
        reg_model.set_allocated_config(config->serialize());
    }

    if (model && model->getRootItem()) {
        SerializationContext context;
        context.serialize(model->getRootItem());
        reg_model << context;
    }

    google::protobuf::Timestamp timestamp;
    timestamp.set_seconds(time(nullptr));
    timestamp.set_nanos(0);
    *(reg_model.mutable_last_updated()) = timestamp;

    bool save_ok = false;
    if (filepath.endsWith(".rmb", Qt::CaseInsensitive)) {
        save_ok = reg_model.SerializeToFileDescriptor(fd);
        ::close(fd);
    } else {
        google::protobuf::io::FileOutputStream fileOutput(fd);
        fileOutput.SetCloseOnDelete(true);
        save_ok = google::protobuf::TextFormat::Print(reg_model, &fileOutput);
    }

    if (!save_ok) {
        result.success = false;
        result.errorMessage = QString("Failed to serialize Protobuf model to %1").arg(filepath);
        return result;
    }

    result.success = true;
    return result;
}
