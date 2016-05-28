//
//  AppDelegate.h
//  IceStar
//
//  Created by Andreas Müller on 31.01.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Connection.h"
#import "ServerTableViewDataSource.h"
#import <Ice/Ice.h>
#import <device.h>


@interface AppDelegate : UIResponder <UIApplicationDelegate> {
    Connection *connection;
    NSNetServiceBrowser *servicebrowser;
}

@property (strong,nonatomic) UIWindow *window;
@property (strong,readwrite) Connection *connection;
@property (strong,readwrite) id<ICECommunicator> communicator;
@property (strong,readwrite) ServerTableViewDataSource *discover;

@end
