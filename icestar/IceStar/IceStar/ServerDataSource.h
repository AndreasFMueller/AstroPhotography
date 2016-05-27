//
//  ServerDataSource.h
//  IceStar
//
//  Created by Andreas Müller on 25.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ServerInfo.h"

@interface ServerDataSource : NSObject

- (NSInteger)count;
- (void)addServer: (ServerInfo *)server;

- (ServerInfo *)serverAtIndex:(NSInteger)index;
- (ServerInfo *)serverWithName:(NSString *)servername;

- (void)removeServerAtIndex:(NSInteger)index;
- (void)removeServerNamed:(NSString *)servername;

- (NSInteger)indexForServerNamed: (NSString *)servername;
- (BOOL)hasServerName: (NSString *)servername;

- (NSIndexPath *)indexPathForServer: (ServerInfo *)server;

@end
