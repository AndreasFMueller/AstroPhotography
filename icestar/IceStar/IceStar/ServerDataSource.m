//
//  ServerDataSource.m
//  IceStar
//
//  Created by Andreas Müller on 25.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "ServerDataSource.h"

@interface ServerDataSource ()
@property (nonatomic, strong) NSMutableArray *servers;
@end

@implementation ServerDataSource

- (id)init {
    if (self) {
        _servers = [[NSMutableArray alloc] init];
    }
    return self;
}

- (NSInteger)numberOfServices {
    return [_servers count];
}

- (void)addServer: (ServerInfo *)server {
    if ([self numberOfServices] == 0) {
        [_servers addObject: server];
        return;
    }
    for (int i = 0; i < [_servers count]; i++) {
        ServerInfo  *current = [self serverAtIndex: i];
        NSLog(@"comparing %@ with %@", current.servicename, server.servicename);
        if (NSOrderedDescending == [current.servicename compare: server.servicename]) {
            NSLog(@"inserting server %@ at position %d", server.servicename, i);
            [_servers insertObject: server atIndex: i];
            return;
        }
    }
    [_servers addObject: server];
}

- (ServerInfo *)serverAtIndex:(NSInteger)index {
    return (ServerInfo *)[_servers objectAtIndex: index];
}

- (void)removeServerAtIndex:(NSInteger)index {
    [_servers removeObjectAtIndex: index];
}

- (void)removeServerNamed:(NSString *)servicename {
    if (![self hasServerName: servicename]) {
        NSLog(@"no service %@", servicename);
        return;
    }
    NSInteger index = [self indexForServerNamed: servicename];
    [self removeServerAtIndex: index];
    NSLog(@"service removed at index %ld", (long)index);
}

- (ServerInfo *)serverWithName:(NSString *)servicename {
    for (int i = 0; i < [self numberOfServices]; i++) {
        ServerInfo  *server = [self serverAtIndex: i];
        if ([server.servicename isEqualToString: servicename]) {
            return server;
        }
    }
    return nil;
}

- (NSIndexPath *)indexPathForServer: (ServerInfo *)server {
    for (int i = 0; i < [self numberOfServices]; i++) {
        if ([self serverAtIndex:i] == server) {
            return [[NSIndexPath alloc] initWithIndex: i];
        }
    }
    return nil;
}

- (BOOL)hasServerName:(NSString *)servicename {
    for (int i = 0; i < [self numberOfServices]; i++) {
        if ([[self serverAtIndex:i].servicename isEqualToString: servicename]) {
            return YES;
        }
    }
    return NO;
}

- (NSInteger)indexForServerNamed: (NSString *)servicename {
    for (int i = 0; i < [self numberOfServices]; i++) {
        if ([[self serverAtIndex:i].servicename isEqualToString: servicename]) {
            return i;
        }
    }
    return -1;
}



@end
