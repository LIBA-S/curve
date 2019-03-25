/*
 * Project: curve
 * File Created: Tuesday, 23rd October 2018 4:57:56 pm
 * Author: tongguangxun
 * Copyright (c) 2018 NetEase
 */

#include "src/client/client_config.h"

namespace curve {
namespace client {
int ClientConfig::Init(const char* configpath) {
    conf_.SetConfigPath(configpath);
    if (!conf_.LoadConfig()) {
        LOG(ERROR) << "Load config failed, config path = " << configpath;
        return -1;
    }

    fileServiceOption_.loginfo.loglevel = conf_.GetIntValue("loglevel", 2);

    fileServiceOption_.ioOpt.ioSplitOpt.ioSplitMaxSize
    = conf_.GetIntValue("ioSplitMaxSize", 64);

    fileServiceOption_.ioOpt.ioSenderOpt.enableAppliedIndexRead
    = conf_.GetIntValue("enableAppliedIndexRead", 1);
    fileServiceOption_.ioOpt.ioSenderOpt.rpcTimeoutMs
    = conf_.GetIntValue("rpcTimeoutMs", 500);
    fileServiceOption_.ioOpt.ioSenderOpt.rpcRetryTimes
    = conf_.GetIntValue("rpcRetryTimes", 3);

    fileServiceOption_.ioOpt.ioSenderOpt.failRequestOpt.opMaxRetry
    = conf_.GetIntValue("opMaxRetry", 3);
    fileServiceOption_.ioOpt.ioSenderOpt.failRequestOpt.opRetryIntervalUs   // NOLINT
    = conf_.GetIntValue("opRetryIntervalUs", 500);

    fileServiceOption_.ioOpt.metaCacheOpt.getLeaderRetry
    = conf_.GetIntValue("getLeaderRetry", 3);
    fileServiceOption_.ioOpt.metaCacheOpt.retryIntervalUs
    = conf_.GetIntValue("retryIntervalUs", 500);

    fileServiceOption_.ioOpt.reqSchdulerOpt.queueCapacity
    = conf_.GetIntValue("queueCapacity", 4096);
    fileServiceOption_.ioOpt.reqSchdulerOpt.threadpoolSize
    = conf_.GetIntValue("threadpoolSize", 2);
    fileServiceOption_.ioOpt.reqSchdulerOpt.ioSenderOpt
    = fileServiceOption_.ioOpt.ioSenderOpt;

    fileServiceOption_.leaseOpt.refreshTimesPerLease
    = conf_.GetIntValue("refreshTimesPerLease", 4);

    fileServiceOption_.metaServerOpt.metaaddrvec.push_back(conf_.
                                            GetStringValue("metaserver_addr"));
    fileServiceOption_.metaServerOpt.rpcTimeoutMs
    = conf_.GetIntValue("rpcTimeoutMs", 500);
    fileServiceOption_.metaServerOpt.rpcRetryTimes
    = conf_.GetIntValue("rpcTimeoutMs", 500);

    return 0;
}

FileServiceOption_t ClientConfig::GetFileServiceOption() {
    return fileServiceOption_;
}
}   // namespace client
}   // namespace curve