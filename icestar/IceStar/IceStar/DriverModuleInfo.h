//
//  ModuleInfo.h
//  IceStar
//
//  Created by Andreas Müller on 12.06.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "device.h"

@interface DriverModuleInfo : NSObject

@property (readonly) NSString   *modulename;
@property (readonly) NSString   *version;
@property (readonly) BOOL   hasDescriptor;
@property (readwrite,strong)   snowstarDriverModulePrx  *drivermodule;

-(id)init: (snowstarDriverModulePrx *)dmprx;

@end
