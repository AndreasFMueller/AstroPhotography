//
//  FirstViewController.m
//  IceStar
//
//  Created by Andreas Müller on 31.01.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <Ice/Ice.h>
#import <module.h>
#import <Foundation/NSAutoreleasePool.h>
#import "FirstViewController.h"

@interface FirstViewController ()

@end

@implementation FirstViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)connect:(id)sender {
    NSLog(@"connect called");
    id<ICECommunicator> communicator = nil;
    @try {
        communicator = [ICEUtil createCommunicator];
        id<ICEObjectPrx>    base = [communicator stringToProxy: @"Modules:default -p 10000"];
        id<snowstarModulesPrx>  modules = [snowstarModulesPrx checkedCast: base];
        if (!modules) {
            [NSException raise: @"InvalidProxy" format:nil];
        }
        int n = [modules numberOfModules];
        NSLog(@"number of modules: %d", n);
    } @catch (NSException *ex) {
        NSLog(@"%@", ex);
    }
    @try {
        [communicator destroy];
    } @catch (NSException* ex) {
        NSLog(@"%@", ex);
    }
    
 }

@end
