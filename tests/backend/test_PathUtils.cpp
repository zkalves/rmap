/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QDir>
#include <QFile>
#include "PathUtils.hpp"

class TestPathUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testNormalizeSeparators();
    void testExpandEnvVarsTilde();
    void testExpandEnvVarsPosix();
    void testExpandEnvVarsWindows();
    void testExpandEnvVarsMixed();
    void testToRelativePath();
    void testResolvePathWithBaseDirs();
    void testResolvePathWithEnvVars();
    void testDefaultPathConstants();
    void testDefaultDirectoryDiscovery();
    void testResolvePathWithTemplatesSubdir();
    void testEdgeCasesAndOverloads();
    void testExtendedPathResolutionAndDiscovery();
};

void TestPathUtils::initTestCase()
{
    qputenv("RMAP_TEST_ROOT", "/tmp/rmap_test_root");
    qputenv("RMAP_TEST_SUB", "sub_dir");
    qputenv("RMAP_TEST_FILE", "output.sv");
}

void TestPathUtils::cleanupTestCase()
{
    qunsetenv("RMAP_TEST_ROOT");
    qunsetenv("RMAP_TEST_SUB");
    qunsetenv("RMAP_TEST_FILE");
    QDir("work/test_path_utils").removeRecursively();
}

void TestPathUtils::testNormalizeSeparators()
{
    QString p1 = "foo\\bar\\baz.txt";
    QCOMPARE(PathUtils::normalizeSeparators(p1), QString("foo/bar/baz.txt"));

    QString p2 = "foo///bar//baz.txt";
    QCOMPARE(PathUtils::normalizeSeparators(p2), QString("foo/bar/baz.txt"));

    std::string s1 = "a\\b\\c";
    QCOMPARE(PathUtils::normalizeSeparators(s1), std::string("a/b/c"));
}

void TestPathUtils::testExpandEnvVarsTilde()
{
    QString home = QDir::homePath();
    QCOMPARE(PathUtils::expandEnvVars("~"), home);
    QCOMPARE(PathUtils::expandEnvVars("~/my_folder/file.txt"), home + "/my_folder/file.txt");
    QCOMPARE(PathUtils::expandEnvVars("~\\windows_style\\path.txt"), home + "/windows_style/path.txt");
}

void TestPathUtils::testExpandEnvVarsPosix()
{
    // $VAR syntax
    QCOMPARE(PathUtils::expandEnvVars("$RMAP_TEST_ROOT/templates"), QString("/tmp/rmap_test_root/templates"));

    // ${VAR} syntax
    QCOMPARE(PathUtils::expandEnvVars("${RMAP_TEST_ROOT}/${RMAP_TEST_SUB}/file.txt"), QString("/tmp/rmap_test_root/sub_dir/file.txt"));

    // Undefined variable expands to empty string
    QCOMPARE(PathUtils::expandEnvVars("$RMAP_NONEXISTENT_VAR_12345/tail.txt"), QString("/tail.txt"));
}

void TestPathUtils::testExpandEnvVarsWindows()
{
    // %VAR% syntax
    QCOMPARE(PathUtils::expandEnvVars("%RMAP_TEST_ROOT%/test/%RMAP_TEST_FILE%"), QString("/tmp/rmap_test_root/test/output.sv"));
}

void TestPathUtils::testExpandEnvVarsMixed()
{
    QCOMPARE(PathUtils::expandEnvVars("$RMAP_TEST_ROOT/%RMAP_TEST_SUB%/${RMAP_TEST_FILE}"), QString("/tmp/rmap_test_root/sub_dir/output.sv"));
}

void TestPathUtils::testToRelativePath()
{
    QString baseDir = "/home/user/project";
    QString absTarget = "/home/user/project/templates/c/reg_map.h.inja";

    QString rel = PathUtils::toRelativePath(absTarget, baseDir);
    QCOMPARE(rel, QString("./templates/c/reg_map.h.inja"));

    QString parentTarget = "/home/user/other/file.txt";
    QString relParent = PathUtils::toRelativePath(parentTarget, baseDir);
    QCOMPARE(relParent, QString("../other/file.txt"));

    // Already relative paths remain relative
    QCOMPARE(PathUtils::toRelativePath("./templates/c/reg_map.h.inja", baseDir), QString("./templates/c/reg_map.h.inja"));
    QCOMPARE(PathUtils::toRelativePath("templates/c/reg_map.h.inja", baseDir), QString("templates/c/reg_map.h.inja"));

    // Environment variables are left untouched
    QCOMPARE(PathUtils::toRelativePath("$PROJECT_ROOT/templates/t.inja", baseDir), QString("$PROJECT_ROOT/templates/t.inja"));
    QCOMPARE(PathUtils::toRelativePath("%PROJECT_ROOT%/templates/t.inja", baseDir), QString("%PROJECT_ROOT%/templates/t.inja"));

    // Empty input returns empty
    QCOMPARE(PathUtils::toRelativePath("", baseDir), QString(""));
}

void TestPathUtils::testResolvePathWithBaseDirs()
{
    // Test resolving relative path against primary base dir
    // We create a temporary structure in work/test_path_utils
    QDir().mkpath("work/test_path_utils/primary");
    QDir().mkpath("work/test_path_utils/secondary");

    QFile f1("work/test_path_utils/primary/tmpl_prim.inja");
    if (f1.open(QIODevice::WriteOnly)) { f1.write("primary"); f1.close(); }

    QFile f2("work/test_path_utils/secondary/tmpl_sec.inja");
    if (f2.open(QIODevice::WriteOnly)) { f2.write("secondary"); f2.close(); }

    QString primDir = QDir("work/test_path_utils/primary").absolutePath();
    QString secDir  = QDir("work/test_path_utils/secondary").absolutePath();

    // Resolves in primary
    QString r1 = PathUtils::resolvePath("tmpl_prim.inja", primDir, secDir);
    QVERIFY(r1.contains("tmpl_prim.inja"));
    QVERIFY(QFile::exists(r1));

    // Resolves in secondary
    QString r2 = PathUtils::resolvePath("tmpl_sec.inja", primDir, secDir);
    QVERIFY(r2.contains("tmpl_sec.inja"));
    QVERIFY(QFile::exists(r2));

    // Non-existent target file constructs relative to primary
    QString r3 = PathUtils::resolvePath("non_existent.txt", primDir, secDir);
    QCOMPARE(r3, PathUtils::normalizeSeparators(primDir + "/non_existent.txt"));
}

void TestPathUtils::testResolvePathWithEnvVars()
{
    QDir().mkpath("work/test_path_utils/env_dir");
    QFile f("work/test_path_utils/env_dir/env_target.txt");
    if (f.open(QIODevice::WriteOnly)) { f.write("ok"); f.close(); }

    QString absEnvDir = QDir("work/test_path_utils/env_dir").absolutePath();
    qputenv("RMAP_TEST_DIR", absEnvDir.toLocal8Bit());

    QString res = PathUtils::resolvePath("$RMAP_TEST_DIR/env_target.txt");
    QVERIFY(QFile::exists(res));
    QCOMPARE(res, PathUtils::normalizeSeparators(absEnvDir + "/env_target.txt"));

    qunsetenv("RMAP_TEST_DIR");
}

void TestPathUtils::testDefaultPathConstants()
{
    QCOMPARE(PathUtils::DEFAULT_OUTPUT_DIR, "./work");
    QCOMPARE(PathUtils::DEFAULT_TEMPLATES_DIR, "./templates");
    QCOMPARE(PathUtils::defaultOutputDir(), QString("./work"));
    QCOMPARE(PathUtils::defaultTemplatesDir(), QString("./templates"));
}

void TestPathUtils::testDefaultDirectoryDiscovery()
{
    // 1. In source repository, defaultTemplatesDir() resolves to ./templates
    QCOMPARE(PathUtils::defaultTemplatesDir(), QString("./templates"));
    QCOMPARE(PathUtils::defaultExamplesDir(), QString("./examples"));
    QCOMPARE(PathUtils::defaultDocsDir(), QString("./docs"));

    // 2. Environment variable overrides take precedence
    qputenv("RMAP_TEMPLATES_DIR", "/custom/installed/templates");
    QCOMPARE(PathUtils::defaultTemplatesDir(), QString("/custom/installed/templates"));
    qunsetenv("RMAP_TEMPLATES_DIR");

    qputenv("RMAP_EXAMPLES_DIR", "/custom/installed/examples");
    QCOMPARE(PathUtils::defaultExamplesDir(), QString("/custom/installed/examples"));
    qunsetenv("RMAP_EXAMPLES_DIR");

    qputenv("RMAP_DOCS_DIR", "/custom/installed/docs");
    QCOMPARE(PathUtils::defaultDocsDir(), QString("/custom/installed/docs"));
    qunsetenv("RMAP_DOCS_DIR");
}

void TestPathUtils::testResolvePathWithTemplatesSubdir()
{
    // When secondary base dir is a templates folder (e.g. /opt/share/rmap/templates)
    // and path starts with "templates/c/reg_map.h.inja", it should resolve correctly to
    // /opt/share/rmap/templates/c/reg_map.h.inja (stripping redundant "templates/" prefix)
    QDir().mkpath("work/test_path_utils/share_tmpl/c");
    QFile f("work/test_path_utils/share_tmpl/c/reg_map.h.inja");
    if (f.open(QIODevice::WriteOnly)) { f.write("tmpl"); f.close(); }

    QString tmplDir = QDir("work/test_path_utils/share_tmpl").absolutePath();
    // Rename/simulate ending with templates
    QDir().mkpath("work/test_path_utils/templates/c");
    QFile f2("work/test_path_utils/templates/c/reg_map.h.inja");
    if (f2.open(QIODevice::WriteOnly)) { f2.write("tmpl2"); f2.close(); }
    QString shareTmplDir = QDir("work/test_path_utils/templates").absolutePath();

    QString resolved = PathUtils::resolvePath("templates/c/reg_map.h.inja", "", shareTmplDir);
    QVERIFY(QFile::exists(resolved));
    QCOMPARE(resolved, PathUtils::normalizeSeparators(shareTmplDir + "/c/reg_map.h.inja"));

    QString resolvedRel = PathUtils::resolvePath("./templates/c/reg_map.h.inja", "", shareTmplDir);
    QVERIFY(QFile::exists(resolvedRel));
    QCOMPARE(resolvedRel, PathUtils::normalizeSeparators(shareTmplDir + "/c/reg_map.h.inja"));
}

void TestPathUtils::testEdgeCasesAndOverloads()
{
    // UNC paths
    QCOMPARE(PathUtils::normalizeSeparators("//server/share/file.txt"), QString("//server/share/file.txt"));
    QCOMPARE(PathUtils::normalizeSeparators("///server/share/file.txt"), QString("/server/share/file.txt"));

    // Empty paths
    QCOMPARE(PathUtils::expandEnvVars(""), QString(""));
    QCOMPARE(PathUtils::expandEnvVars(std::string("")), std::string(""));
    QCOMPARE(PathUtils::toRelativePath(""), QString(""));
    QCOMPARE(PathUtils::toRelativePath(std::string("")), std::string(""));
    QCOMPARE(PathUtils::resolvePath(""), QString(""));
    QCOMPARE(PathUtils::resolvePath(std::string("")), std::string(""));

    // Base dir pointing to file
    QDir().mkpath("work/test_path_utils");
    QFile tmpFile("work/test_path_utils/base_file.txt");
    if (tmpFile.open(QIODevice::WriteOnly)) { tmpFile.close(); }
    QString absBase = QFileInfo("work/test_path_utils/base_file.txt").absoluteFilePath();
    QString absTarget = QFileInfo("work/test_path_utils/target.txt").absoluteFilePath();
    QString relFromFile = PathUtils::toRelativePath(absTarget, absBase);
    QCOMPARE(relFromFile, QString("./target.txt"));

    // resolvePath with absolute path that does not exist
    QString nonExAbs = "/this/path/does/not/exist/foo.txt";
    QCOMPARE(PathUtils::resolvePath(nonExAbs), nonExAbs);

    // resolvePath fallback with no base dirs
    QString cwdFallback = PathUtils::resolvePath("nonexistent_target_123.txt");
    QCOMPARE(cwdFallback, PathUtils::normalizeSeparators(QDir::current().filePath("nonexistent_target_123.txt")));

    // resolvePath fallback with secondary base dir ending with templates
    QString shareTmplDir = QDir("work/test_path_utils/templates").absolutePath();
    QString tmplFallback = PathUtils::resolvePath("templates/sub/new_file.txt", "", shareTmplDir);
    QCOMPARE(tmplFallback, PathUtils::normalizeSeparators(shareTmplDir + "/sub/new_file.txt"));
    QString tmplFallback2 = PathUtils::resolvePath("./templates/sub/new_file2.txt", "", shareTmplDir);
    QCOMPARE(tmplFallback2, PathUtils::normalizeSeparators(shareTmplDir + "/sub/new_file2.txt"));

    // std::string overloads
    std::string s_in = "foo/bar";
    QCOMPARE(PathUtils::expandEnvVars(s_in), s_in);
    QCOMPARE(PathUtils::toRelativePath(s_in, std::string("")), s_in);
    QCOMPARE(PathUtils::resolvePath(s_in, std::string(""), std::string("")), PathUtils::resolvePath(QString::fromStdString(s_in)).toStdString());

    // const char* inline overloads
    const char* c_in = "foo/bar";
    QCOMPARE(PathUtils::expandEnvVars(c_in), QString(c_in));
    QCOMPARE(PathUtils::toRelativePath(c_in), QString(c_in));
    QCOMPARE(PathUtils::resolvePath(c_in), PathUtils::resolvePath(QString(c_in)));
    QCOMPARE(PathUtils::normalizeSeparators(c_in), QString(c_in));

    // const char* null checks
    QCOMPARE(PathUtils::expandEnvVars(static_cast<const char*>(nullptr)), QString(""));
    QCOMPARE(PathUtils::toRelativePath(static_cast<const char*>(nullptr)), QString(""));
    QCOMPARE(PathUtils::resolvePath(static_cast<const char*>(nullptr)), QString(""));
    QCOMPARE(PathUtils::normalizeSeparators(static_cast<const char*>(nullptr)), QString(""));
}

void TestPathUtils::testExtendedPathResolutionAndDiscovery()
{
    // 1. Primary base dir template resolution stripping "templates/"
    QString shareTmplDir = QDir("work/test_path_utils/templates").absolutePath();
    QDir().mkpath(shareTmplDir + "/c");
    QFile f(shareTmplDir + "/c/reg_map.h.inja");
    if (f.open(QIODevice::WriteOnly)) { f.write("tmpl"); f.close(); }

    QString resPrim = PathUtils::resolvePath("templates/c/reg_map.h.inja", shareTmplDir);
    QVERIFY(QFile::exists(resPrim));
    QCOMPARE(resPrim, PathUtils::normalizeSeparators(shareTmplDir + "/c/reg_map.h.inja"));

    QString resPrimRel = PathUtils::resolvePath("./templates/c/reg_map.h.inja", shareTmplDir);
    QVERIFY(QFile::exists(resPrimRel));
    QCOMPARE(resPrimRel, PathUtils::normalizeSeparators(shareTmplDir + "/c/reg_map.h.inja"));

    // 2. resolvePath fallback with empty primary and valid secondary base dir
    QString outFallback = PathUtils::resolvePath("out_nonexistent.sv", "", "/tmp/secondary_out_base");
    QCOMPARE(outFallback, QString("/tmp/secondary_out_base/out_nonexistent.sv"));

    // 3. Relocatable directory discovery (<appDir>/../share/rmap/...) and relative fallback
    QString origCwd = QDir::currentPath();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir::setCurrent(tempDir.path());

    QString appDir = QCoreApplication::applicationDirPath();
    QString relShareTmpl = appDir + "/../share/rmap/templates";
    QString relShareEx = appDir + "/../share/rmap/examples";
    QString relShareDoc = appDir + "/../share/doc/rmap";

    QDir().mkpath(relShareTmpl);
    QDir().mkpath(relShareEx);
    QDir().mkpath(relShareDoc);

    QVERIFY(!PathUtils::defaultTemplatesDir().isEmpty());
    QVERIFY(!PathUtils::defaultExamplesDir().isEmpty());
    QVERIFY(!PathUtils::defaultDocsDir().isEmpty());

    QDir(appDir + "/../share").removeRecursively();

    QCOMPARE(PathUtils::defaultTemplatesDir(), QString("./templates"));
    QCOMPARE(PathUtils::defaultExamplesDir(), QString("./examples"));
    QCOMPARE(PathUtils::defaultDocsDir(), QString("./docs"));

    QDir::setCurrent(origCwd);
}

QTEST_MAIN(TestPathUtils)
#include "test_PathUtils.moc"
