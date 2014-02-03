//
//  DeviceListController.h
//  IceStar
//
//  Created by Andreas Müller on 02.02.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <device.h>

@interface DeviceListController : UITableViewController {
    snowstarMutableDeviceNameList *devicelist;
}

- (id)initWithDeviceType: (ICEInt)type;
- (NSString *)nameAtIndex: (int)index;

@end
