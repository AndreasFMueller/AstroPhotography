//
//  ServerInfo.m
//  IceStar
//
//  Created by Andreas Müller on 25.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "ServerInfo.h"
#include <netinet/in.h>
#include <arpa/inet.h>

@implementation ServerInfo

- (void)netServiceWillResolve:(NSNetService *)sender {
    NSLog(@"resolution of %@ started", sender.name);
}

- (void)netServiceDidResolveAddress:(NSNetService *)sender {
    NSLog(@"resolution complete: %@", sender.name);
    NSArray<NSData *> *a = sender.addresses;
    if ([a count] == 0) {
        return;
    }
    self.hostname = sender.hostName;
    [self processAddress: [a objectAtIndex: 0]];
    [self processTXTRecordData: sender.TXTRecordData];
}

- (void)netService:(NSNetService *)sender didNotResolve:(NSDictionary<NSString *, NSNumber *> *)errorDict {
    NSLog(@"resolution for %@ failed", sender.name);
}

- (void)netService:(NSNetService *)sender didUpdateTXTRecordData:(NSData *)data {
    NSLog(@"TXT for %@ updated", sender.name);
    [self processTXTRecordData: data];
}

- (void)processAddress: (NSData *)a {
    NSLog(@"adding IP address");
    struct sockaddr_in  sin;
    [a getBytes: &sin length: sizeof(sin)];
    _port = ntohs(sin.sin_port);
    _address = sin.sin_addr.s_addr;
    NSLog(@"address: %s", inet_ntoa(sin.sin_addr));
}

- (void)processTXTRecordData: (NSData *)data {
    NSLog(@"processing TXT record");
    _devices = NO;
    _instruments = NO;
    _tasks = NO;
    _guiding = NO;
    _focusing = NO;
    _images = NO;
    _repository = NO;
    NSDictionary<NSString *, NSData *>  *dict = [NSNetService dictionaryFromTXTRecordData: data];
    NSMutableArray  *allKey = [[dict allKeys] mutableCopy];
    for (NSString *key in allKey) {
        NSLog(@"key: %@", key);
        if ([key isEqualToString: @"devices"]) {
            _devices = YES;
        }
        if ([key isEqualToString: @"instruments"]) {
            _instruments = YES;
        }
        if ([key isEqualToString: @"tasks"]) {
            _tasks = YES;
        }
        if ([key isEqualToString: @"guiding"]) {
            _guiding = YES;
        }
        if ([key isEqualToString: @"focusing"]) {
            _focusing = YES;
        }
        if ([key isEqualToString: @"images"]) {
            _images = YES;
        }
        if ([key isEqualToString: @"repository"]) {
            _repository = YES;
        }
    }
}

@end
