//
//  Discover.m
//  IceStar
//
//  Created by Andreas Müller on 25.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "Discover.h"

@implementation Discover

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser
            didFindDomain:(NSString *)domainName
               moreComing:(BOOL)moreDomainsComing {
    NSLog(@"new domain: %@", domainName);
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser
           didFindService:(NSNetService *)netService
               moreComing:(BOOL)moreServicesComing {
    NSLog(@"new service: %@, type=%@, domain=%@, hostname=%@", netService.name, netService.type, netService.domain, netService.hostName);
    ServerInfo  *serverinfo = [[ServerInfo alloc] init];
    serverinfo.servicename = netService.name;
    serverinfo.domain = netService.domain;
    serverinfo.netservice = netService;
    if (netService.addresses.count > 0) {
        [serverinfo processAddress: [netService.addresses objectAtIndex: 0]];
    }
    [self addServer: serverinfo];
    [netService setDelegate: serverinfo];
    [netService resolveWithTimeout: 10.0];
//    [netService startMonitoring];
    NSLog(@"resolution initiated");
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser
         didRemoveService:(NSNetService *)netService
               moreComing:(BOOL)moreComing {
    NSLog(@"server %@ removed", netService.name);
    [self removeServerNamed: netService.name];
}

@end
