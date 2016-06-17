//
//  ModuleInfo.m
//  IceStar
//
//  Created by Andreas Müller on 12.06.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "DriverModuleInfo.h"

@implementation DriverModuleInfo

-(id)init: (snowstarDriverModulePrx *)dmprx {
    if (self = [super init]) {
        _drivermodule = dmprx;
        _modulename = [dmprx getName];
        _hasDescriptor = [dmprx hasLocator];
        _version = [dmprx getVersion];
    }
    return self;
}

@end
