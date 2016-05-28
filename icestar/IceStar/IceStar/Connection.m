//
//  Connection.m
//  IceStar
//
//  Created by Andreas Müller on 02.02.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import "Connection.h"
#import "AppDelegate.h"

@implementation Connection

@synthesize devices;

- (id)init {
    if (self = [super init]) {
        // create connection preferences
        NSUserDefaults  *defaults = [NSUserDefaults standardUserDefaults];
        long port = [defaults integerForKey: @"port_preference"];
        if (0 == port) {
            port = 10000;
        }
        NSString    *host = [defaults stringForKey: @"host_preference"];
        if (nil == host) {
            host = @"localhost";
        }
        
        // get the remote host and portnumber from the default settings
        AppDelegate     *appdelegate = [[UIApplication sharedApplication] delegate];
        
        // build the name for the remote device
        NSString    *name = [NSString stringWithFormat: @"Devices:default -h %@ -p %ld", host, port];
        NSLog(@"connecting to %@", name);
        
        // get the remote object
        id<ICEObjectPrx>    base = [appdelegate.communicator stringToProxy: name];
        devices = [snowstarDevicesPrx checkedCast: base];
    }
    return self;
}

+ (snowstarDevicesPrx *)devicesProxy: (ServerInfo *)serverinfo {
    NSString    *name = [NSString stringWithFormat: @"Devices:default -h %@ -p %ld", serverinfo.hostname, (long)serverinfo.port];
    AppDelegate     *appdelegate = [[UIApplication sharedApplication] delegate];

    id<ICEObjectPrx>    base = [appdelegate.communicator stringToProxy: name];

    return [snowstarDevicesPrx checkedCast: base];
}

+ (snowstarInstrumentsPrx *)instrumentsProxy: (ServerInfo *)serverinfo {
    if (!serverinfo.instruments) {
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle: @"Not supported" message: @"The server does not support the Instruments service" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
        [alert show];
        return nil;
    }
    NSString    *name = [NSString stringWithFormat: @"Instruments:default -h %@ -p %ld", serverinfo.hostname, (long)serverinfo.port];
    NSLog(@"connection string: %@", name);
    AppDelegate     *appdelegate = [[UIApplication sharedApplication] delegate];
    
    id<ICEObjectPrx>    base = [appdelegate.communicator stringToProxy: name];
    
    return [snowstarInstrumentsPrx checkedCast: base];
}


@end
