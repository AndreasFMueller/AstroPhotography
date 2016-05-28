//
//  ServerInfo.h
//  IceStar
//
//  Created by Andreas Müller on 25.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ServerInfo : NSObject<NSNetServiceDelegate>

@property (nonatomic,strong)    NSString    *servicename;
@property (nonatomic,strong)    NSString    *type;
@property (nonatomic,strong)    NSString    *domain;
@property (nonatomic,strong)    NSString    *hostname;
@property (nonatomic) in_addr_t address;
@property (nonatomic) int   port;
@property (nonatomic) int   sslport;
@property (readonly) BOOL   instruments;
@property (readonly) BOOL   devices;
@property (readonly) BOOL   tasks;
@property (readonly) BOOL   guiding;
@property (readonly) BOOL   focusing;
@property (readonly) BOOL   images;
@property (readonly) BOOL   repository;
@property (nonatomic,strong)    NSNetService    *netservice;

- (void)netServiceWillResolve:(NSNetService *)sender;
- (void)netServiceDidResolveAddress:(NSNetService *)sender;
- (void)netService:(NSNetService *)sender
     didNotResolve:(NSDictionary<NSString *,
                    NSNumber *> *)errorDict;
- (void)netService:(NSNetService *)sender
    didUpdateTXTRecordData:(NSData *)data;

- (void)processAddress: (NSData *)a;
- (void)processTXTRecordData: (NSData *)data;

@end
