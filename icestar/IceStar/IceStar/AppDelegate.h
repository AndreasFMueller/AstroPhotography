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

@interface AppDelegate : UIResponder <UIApplicationDelegate> {
    Connection *connection;
    NSNetServiceBrowser *servicebrowser;
//    ServerTableViewDataSource    *discover;
}

@property (strong,nonatomic) UIWindow *window;
@property (strong,readwrite) Connection *connection;
@property (strong,readwrite) ServerTableViewDataSource *discover;

@end
