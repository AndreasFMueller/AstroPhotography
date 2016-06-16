//
//  DevicesDataSource.h
//  IceStar
//
//  Created by Andreas Müller on 12.06.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "device.h"

@interface DevicesDataSource : NSObject <UITableViewDataSource>

@property (readwrite,strong) snowstarDevicesPrx *devices;
@property (readwrite,strong) snowstarDeviceNameList *ao;
@property (readwrite,strong) snowstarDeviceNameList *camera;
@property (readwrite,strong) snowstarDeviceNameList *ccd;
@property (readwrite,strong) snowstarDeviceNameList *cooler;
@property (readwrite,strong) snowstarDeviceNameList *filterwheel;
@property (readwrite,strong) snowstarDeviceNameList *focuser;
@property (readwrite,strong) snowstarDeviceNameList *guiderport;
@property (readwrite,strong) snowstarDeviceNameList *mount;

@property (readwrite) BOOL showAO;
@property (readwrite) BOOL showCamera;
@property (readwrite) BOOL showCcd;
@property (readwrite) BOOL showCooler;
@property (readwrite) BOOL showFilterwheel;
@property (readwrite) BOOL showFocuser;
@property (readwrite) BOOL showGuiderport;
@property (readwrite) BOOL showMount;

- (id)init: (snowstarDevicesPrx *) devicesprx;
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView;
- (snowstarDeviceNameList *)nameList: (NSInteger)section;
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section;
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath;
- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section;

@end
