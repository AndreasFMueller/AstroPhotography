//
//  Discover.h
//  IceStar
//
//  Created by Andreas Müller on 25.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ServerDataSource.h"

@interface Discover : ServerDataSource<NSNetServiceBrowserDelegate>

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser
            didFindDomain:(NSString *)domainName
               moreComing:(BOOL)moreDomainsComing;

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser
           didFindService:(NSNetService *)netService
               moreComing:(BOOL)moreServicesComing;

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser
         didRemoveService:(NSNetService *)aNetService
               moreComing:(BOOL)moreComing;

@end
