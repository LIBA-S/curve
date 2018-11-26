/*
 * Copyright (C) 2018 NetEase Inc. All rights reserved.
 * Project: Curve
 *
 * History:
 *          2018/11/23  Wenyu Zhou   Initial version
 */

#include <gtest/gtest.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <string>

// #define CBD_BACKEND_FAKE

#include "src/client/libcbd.h"

#include "include/client/libcurve.h"
#include "src/client/session.h"
#include "test/client/fake/mock_schedule.h"
#include "test/client/fake/fakeMDS.h"

#define BUFSIZE     4 * 1024
#define FILESIZE    10uL * 1024 * 1024 * 1024

#define filename    "test.img"

void LibcbdLibcurveTestCallback(CurveAioContext* context) {
    context->op = LIBCURVE_OP_MAX;
}

class TestLibcbdLibcurve : public ::testing::Test {
 public:
    void SetUp() {
        mds_ = new FakeMDS(filename);

        /*** init mds service ***/
        mds_->Initialize();
        mds_->StartService();
        mds_->CreateCopysetNode();

        int64_t t0 = butil::monotonic_time_ms();
        CreateFileErrorType e;
        for (;;) {
            e = CreateFile(filename, FILESIZE);
            if (e == FILE_CREATE_OK || e == FILE_ALREADY_EXISTS) {
                LOG(INFO) << "Created file for test.";
                break;
            }

            int64_t t1 = butil::monotonic_time_ms();
            // Set timeout to 10 seconds
            if (t1 - t0 > 10 * 1000) {
                LOG(ERROR) << "Timed out retrying of creating file.";
                break;
            }

            LOG(INFO) << "Failed to create file, retrying again.";
            usleep(100 * 1000);
        }
        ASSERT_TRUE(e == FILE_CREATE_OK || e == FILE_ALREADY_EXISTS);
    }

    void TearDown() {
        mds_->UnInitialize();

        delete mds_;
    }

 private:
    FakeMDS* mds_;
};

TEST_F(TestLibcbdLibcurve, InitTest) {
    int ret;
    CurveOptions opt;

    memset(&opt, 0, sizeof(opt));

    // testing with no conf specified
    ret = cbd_lib_init(&opt);
    ASSERT_NE(ret, 0);
    ret = cbd_lib_fini();
    ASSERT_EQ(ret, 0);

    // testing with conf specified
    opt.conf = "invalid_conf_path";
    ret = cbd_lib_init(&opt);
    ASSERT_EQ(ret, 0);
    ret = cbd_lib_fini();
    ASSERT_EQ(ret, 0);
}

TEST_F(TestLibcbdLibcurve, ReadWriteTest) {
    int ret;
    int fd;
    int i;
    char buf[BUFSIZE];
    CurveOptions opt;

    memset(&opt, 0, sizeof(opt));
    memset(buf, 'a', BUFSIZE);

    opt.conf = "invalid_conf_path";
    ret = cbd_lib_init(&opt);
    ASSERT_EQ(ret, 0);

    fd = cbd_lib_open(filename);
    ASSERT_GE(fd, 0);

    uint64_t size = cbd_lib_filesize(filename);
    ASSERT_EQ(size, FILESIZE);

    ret = cbd_lib_pwrite(fd, buf, 0, BUFSIZE);
    ASSERT_EQ(ret, BUFSIZE);

    ret = cbd_lib_sync(fd);
    ASSERT_EQ(ret, 0);

    ret = cbd_lib_pread(fd, buf, 0, BUFSIZE);
    ASSERT_EQ(ret, BUFSIZE);

    for (i = 0; i < BUFSIZE; i++) {
        if (buf[i] != 'a') {
            break;
        }
    }
    ASSERT_EQ(i, BUFSIZE);

    ret = cbd_lib_close(fd);
    ASSERT_EQ(ret, 0);

    ret = cbd_lib_fini();
    ASSERT_EQ(ret, 0);
}

TEST_F(TestLibcbdLibcurve, AioReadWriteTest) {
    int ret;
    int fd;
    int i;
    char buf[BUFSIZE];
    CurveOptions opt;
    CurveAioContext aioCtx;

    aioCtx.buf = buf;
    aioCtx.offset = 0;
    aioCtx.length = BUFSIZE;
    aioCtx.cb = LibcbdLibcurveTestCallback;

    memset(&opt, 0, sizeof(opt));
    memset(buf, 'a', BUFSIZE);

    opt.conf = "invalid_conf_path";
    ret = cbd_lib_init(&opt);
    ASSERT_EQ(ret, 0);

    fd = cbd_lib_open(filename);
    ASSERT_GE(fd, 0);

    uint64_t size = cbd_lib_filesize(filename);
    ASSERT_EQ(size, FILESIZE);

    aioCtx.op = LIBCURVE_OP_WRITE;
    ret = cbd_lib_aio_pwrite(fd, &aioCtx);
    ASSERT_EQ(ret, 0);

    while (aioCtx.op == LIBCURVE_OP_WRITE) {
        usleep(10 * 1000);
    }

    ret = cbd_lib_sync(fd);
    ASSERT_EQ(ret, 0);

    aioCtx.op = LIBCURVE_OP_READ;
    ret = cbd_lib_aio_pread(fd, &aioCtx);
    ASSERT_EQ(ret, 0);

    while (aioCtx.op == LIBCURVE_OP_READ) {
        usleep(10 * 1000);
    }

    for (i = 0; i < BUFSIZE; i++) {
        if (buf[i] != 'a') {
            break;
        }
    }
    ASSERT_EQ(i, BUFSIZE);

    ret = cbd_lib_close(fd);
    ASSERT_EQ(ret, 0);

    ret = cbd_lib_fini();
    ASSERT_EQ(ret, 0);
}
