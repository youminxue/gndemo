// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/failing_http_transaction_factory.h"

#include <stdint.h>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "net/base/load_timing_info.h"
#include "net/base/net_error_details.h"
#include "net/http/http_response_info.h"
#include "net/socket/connection_attempts.h"

namespace net {

class AuthCredentials;
class HttpRequestHeaders;
class IOBuffer;
class NetLogWithSource;
class SSLPrivateKey;
class X509Certificate;

namespace {

// A simple class to interpose between the cache and network http layers.
// These transactions can be generated by the FailingHttpTransactionFactory
// to test interactions between cache and network.
class FailingHttpTransaction : public HttpTransaction {
 public:
  explicit FailingHttpTransaction(Error error);
  ~FailingHttpTransaction() override;

  // HttpTransaction
  int Start(const HttpRequestInfo* request_info,
            const CompletionCallback& callback,
            const NetLogWithSource& net_log) override;
  int RestartIgnoringLastError(const CompletionCallback& callback) override;
  int RestartWithCertificate(scoped_refptr<X509Certificate> client_cert,
                             scoped_refptr<SSLPrivateKey> client_private_key,
                             const CompletionCallback& callback) override;
  int RestartWithAuth(const AuthCredentials& credentials,
                      const CompletionCallback& callback) override;
  bool IsReadyToRestartForAuth() override;
  int Read(IOBuffer* buf,
           int buf_len,
           const CompletionCallback& callback) override;
  void StopCaching() override;
  bool GetFullRequestHeaders(HttpRequestHeaders* headers) const override;
  int64_t GetTotalReceivedBytes() const override;
  int64_t GetTotalSentBytes() const override;
  void DoneReading() override;
  const HttpResponseInfo* GetResponseInfo() const override;
  LoadState GetLoadState() const override;
  void SetQuicServerInfo(QuicServerInfo* quic_server_info) override;
  bool GetLoadTimingInfo(LoadTimingInfo* load_timing_info) const override;
  bool GetRemoteEndpoint(IPEndPoint* endpoint) const override;
  void PopulateNetErrorDetails(NetErrorDetails* details) const override;
  void SetPriority(RequestPriority priority) override;
  void SetWebSocketHandshakeStreamCreateHelper(
      WebSocketHandshakeStreamBase::CreateHelper* create_helper) override;
  void SetBeforeNetworkStartCallback(
      const BeforeNetworkStartCallback& callback) override;
  void SetBeforeHeadersSentCallback(
      const BeforeHeadersSentCallback& callback) override;
  int ResumeNetworkStart() override;
  void GetConnectionAttempts(ConnectionAttempts* out) const override;
  void SetRequestHeadersCallback(RequestHeadersCallback) override {}
  void SetResponseHeadersCallback(ResponseHeadersCallback) override {}

 private:
  Error error_;
  HttpResponseInfo response_;
};

FailingHttpTransaction::FailingHttpTransaction(Error error) : error_(error) {
  DCHECK_LT(error, OK);
}

FailingHttpTransaction::~FailingHttpTransaction() = default;

int FailingHttpTransaction::Start(const HttpRequestInfo* request_info,
                                  const CompletionCallback& callback,
                                  const NetLogWithSource& net_log) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                base::Bind(callback, error_));
  return ERR_IO_PENDING;
}

int FailingHttpTransaction::RestartIgnoringLastError(
    const CompletionCallback& callback)  {
  return ERR_FAILED;
}

int FailingHttpTransaction::RestartWithCertificate(
    scoped_refptr<X509Certificate> client_cert,
    scoped_refptr<SSLPrivateKey> client_private_key,
    const CompletionCallback& callback) {
  return ERR_FAILED;
}

int FailingHttpTransaction::RestartWithAuth(
    const AuthCredentials& credentials,
    const CompletionCallback& callback)  {
  return ERR_FAILED;
}

bool FailingHttpTransaction::IsReadyToRestartForAuth()  {
  return false;
}

int FailingHttpTransaction::Read(IOBuffer* buf, int buf_len,
                                 const CompletionCallback& callback)  {
  NOTREACHED();
  return ERR_FAILED;
}

void FailingHttpTransaction::StopCaching()  {}

bool FailingHttpTransaction::GetFullRequestHeaders(
    HttpRequestHeaders* headers) const  {
  return false;
}

int64_t FailingHttpTransaction::GetTotalReceivedBytes() const {
  return 0;
}

int64_t FailingHttpTransaction::GetTotalSentBytes() const {
  return 0;
}

void FailingHttpTransaction::DoneReading()  {
  NOTREACHED();
}

const HttpResponseInfo* FailingHttpTransaction::GetResponseInfo() const  {
  return &response_;
}

LoadState FailingHttpTransaction::GetLoadState() const  {
  return LOAD_STATE_IDLE;
}

void FailingHttpTransaction::SetQuicServerInfo(
    QuicServerInfo* quic_server_info) {
}

bool FailingHttpTransaction::GetLoadTimingInfo(
    LoadTimingInfo* load_timing_info) const  {
  return false;
}

bool FailingHttpTransaction::GetRemoteEndpoint(IPEndPoint* endpoint) const {
  return false;
}

void FailingHttpTransaction::PopulateNetErrorDetails(
    NetErrorDetails* /*details*/) const {
  return;
}

void FailingHttpTransaction::SetPriority(RequestPriority priority)  {}

void FailingHttpTransaction::SetWebSocketHandshakeStreamCreateHelper(
    WebSocketHandshakeStreamBase::CreateHelper* create_helper)  {
  NOTREACHED();
}

void FailingHttpTransaction::SetBeforeNetworkStartCallback(
    const BeforeNetworkStartCallback& callback)  {
}

void FailingHttpTransaction::SetBeforeHeadersSentCallback(
    const BeforeHeadersSentCallback& callback) {}

int FailingHttpTransaction::ResumeNetworkStart()  {
  NOTREACHED();
  return ERR_FAILED;
}

void FailingHttpTransaction::GetConnectionAttempts(
    ConnectionAttempts* out) const {
  NOTIMPLEMENTED();
}

}  // namespace

FailingHttpTransactionFactory::FailingHttpTransactionFactory(
    HttpNetworkSession* session,
    Error error) : session_(session), error_(error) {
  DCHECK_LT(error, OK);
}

FailingHttpTransactionFactory::~FailingHttpTransactionFactory() = default;

// HttpTransactionFactory:
int FailingHttpTransactionFactory::CreateTransaction(
    RequestPriority priority,
    std::unique_ptr<HttpTransaction>* trans) {
  trans->reset(new FailingHttpTransaction(error_));
  return OK;
}

HttpCache* FailingHttpTransactionFactory::GetCache() {
  return NULL;
}

HttpNetworkSession* FailingHttpTransactionFactory::GetSession() {
  return session_;
}

}  // namespace net
