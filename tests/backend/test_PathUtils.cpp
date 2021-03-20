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
    QString absTarget = "/home/user/project/templates/reg_map.h.inja";

    QString rel = PathUtils::toRelativePath(absTarget, baseDir);
    QCOMPARE(rel, QString("./templates/reg_map.h.inja"));

    QString parentTarget = "/home/user/other/file.txt";
    QString relParent = PathUtils::toRelativePath(parentTarget, baseDir);
    QCOMPARE(relParent, QString("../other/file.txt"));

    // Already relative paths remain relative
    QCOMPARE(PathUtils::toRelativePath("./templates/reg_map.h.inja", baseDir), QString("./templates/reg_map.h.inja"));
    QCOMPARE(PathUtils::toRelativePath("templates/reg_map.h.inja", baseDir), QString("templates/reg_map.h.inja"));

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

QTEST_MAIN(TestPathUtils)
#include "test_PathUtils.moc"
