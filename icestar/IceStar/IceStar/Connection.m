//
//  Connection.m
//  IceStar
//
//  Created by Andreas Müller on 02.02.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import "Connection.h"

@implementation Connection

@synthesize devices;

- (id)init {
    if (self = [super init]) {
        // create connection preferences
        NSUserDefaults  *defaults = [NSUserDefaults standardUserDefaults];
        int port = [defaults integerForKey: @"port_preference"];
        if (0 == port) {
            port = 10000;
        }
        NSString    *host = [defaults stringForKey: @"host_preference"];
        if (nil == host) {
            host = @"localhost";
        }
        
        // get the remote host and portnumber from the default settings
        communicator = [ICEUtil createCommunicator];
        
        // build the name for the remote device
        NSString    *name = [NSString stringWithFormat: @"Devices:default -h %@ -p %d", host, port];
        NSLog(@"connecting to %@", name);
        
        // get the remote object
        id<ICEObjectPrx>    base = [communicator stringToProxy: name];
        devices = [snowstarDevicesPrx checkedCast: base];
    }
    return self;
}

@end
