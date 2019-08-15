/*
 * Copyright 2018-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Function and includes sourced from https://github.com/facebook/wangle/blob/master/wangle/example/ssl/Server.cpp
 * The only modifications are typedef of "RTaggerPipeline", and replacing "EchoPipeline" with "RTaggerPipeline".
 */

#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/ssl/TLSCredProcessor.h>

typedef wangle::Pipeline<folly::IOBufQueue&,  const char*> RTaggerPipeline;

using namespace wangle;
using namespace folly;

// Init the processor callbacks.  It's fine to do this
// even if nothing is being watched
void initCredProcessorCallbacks(
    ServerBootstrap<RTaggerPipeline>& sb,
    TLSCredProcessor& processor) {
  // set up ticket seed callback
  processor.addTicketCallback([&](TLSTicketKeySeeds seeds) {
    // update
    sb.forEachWorker([&](Acceptor* acceptor) {
      if (!acceptor) {
        // this condition can happen if the processor triggers before the
        // server is ready / listening
        return;
      }
      auto evb = acceptor->getEventBase();
      if (!evb) {
        return;
      }
      evb->runInEventBaseThread([acceptor, seeds] {
        acceptor->setTLSTicketSecrets(
          seeds.oldSeeds, seeds.currentSeeds, seeds.newSeeds);
        });
      });
  });

  // Reconfigure SSL when we detect cert or CA changes.
  processor.addCertCallback([&] {
    sb.forEachWorker([&](Acceptor* acceptor) {
      if (!acceptor) {
        return;
      }
      auto evb = acceptor->getEventBase();
      if (!evb) {
        return;
      }
      evb->runInEventBaseThread([acceptor] {
        acceptor->resetSSLContextConfigs();
      });
    });
  });
}
