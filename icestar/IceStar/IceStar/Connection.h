//
//  Connection.h
//  IceStar
//
//  Created by Andreas Müller on 02.02.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Ice/Ice.h>
#import <device.h>
#import <instruments.h>
#import "ServerInfo.h"

@interface Connection : NSObject {
    id<ICECommunicator> communicator;
    id<snowstarDevicesPrx>  devices;
}

@property (readonly) id<snowstarDevicesPrx> devices;

- (id)init;

+ (snowstarDevicesPrx *)devicesProxy: (ServerInfo *)serverinfo;
+ (snowstarInstrumentsPrx *)instrumentsProxy: (ServerInfo *)serverinfo;


@end
