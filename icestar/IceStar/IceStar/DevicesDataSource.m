//
//  DevicesDataSource.m
//  IceStar
//
//  Created by Andreas Müller on 12.06.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "DevicesDataSource.h"

@implementation DevicesDataSource

- (id)init: (snowstarDevicesPrx *) devicesprx {
    if (self = [super init]) {
        _devices = devicesprx;
        _ao = [self.devices getDevicelist: snowstarDevAO];
        _camera = [self.devices getDevicelist: snowstarDevCAMERA];
        _ccd = [self.devices getDevicelist: snowstarDevCCD];
        _cooler = [self.devices getDevicelist: snowstarDevCOOLER];
        _filterwheel = [self.devices getDevicelist: snowstarDevFILTERWHEEL];
        _focuser = [self.devices getDevicelist: snowstarDevFOCUSER];
        _guiderport = [self.devices getDevicelist: snowstarDevGUIDERPORT];
        _mount = [self.devices getDevicelist: snowstarDevMOUNT];
        _showAO = NO;
        _showCamera = NO;
        _showCcd = NO;
        _showCooler = NO;
        _showFilterwheel = NO;
        _showFocuser = NO;
        _showGuiderport = NO;
        _showMount = NO;
    }
    return self;
}

- (NSInteger)countSections {
    NSInteger   result = 0;
    if (_showAO) { result++; }
    if (_showCamera) { result++; }
    if (_showCcd) { result++; }
    if (_showCooler) { result++; }
    if (_showFilterwheel) { result++; }
    if (_showFocuser) { result++; }
    if (_showGuiderport) { result++; }
    if (_showMount) { result++; }
    return result;
}

- (snowstarDeviceNameList *)nameList: (NSInteger)section {
    NSInteger   counter = 0;
    if (_showAO) {
        if (counter == section) {
            return _ao;
        }
        counter++;
    }
    if (_showCamera) {
        if (counter == section) {
            return _camera;
        }
        counter++;
    }
    if (_showCcd) {
        if (counter == section) {
            return _ccd;
        }
        counter++;
    }
    if (_showCooler) {
        if (counter == section) {
            return _cooler;
        }
        counter++;
    }
    if (_showFilterwheel) {
        if (counter == section) {
            return _filterwheel;
        }
        counter++;
    }
    if (_showFocuser) {
        if (counter == section) {
            return _focuser;
        }
        counter++;
    }
    if (_showGuiderport) {
        if (counter == section) {
            return _guiderport;
        }
        counter++;
    }
    if (_showMount) {
        if (counter == section) {
            return _mount;
        }
    }
    return nil;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return [self countSections];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self nameList: section] count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    NSString    *devicename = [[self nameList: indexPath.section] objectAtIndex: indexPath.row];
    if (nil == devicename) {
        return nil;
    }
    NSString    *CellIdentifier = @"DevicesCell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    cell.textLabel.text = devicename;
    return cell;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    NSInteger   counter = 0;
    if (_showAO) {
        if (counter == section) {
            return @"Adaptive Optics";
        }
        counter++;
    }
    if (_showCamera) {
        if (counter == section) {
            return @"Camera";
        }
        counter++;
    }
    if (_showCcd) {
        if (counter == section) {
            return @"CCD";
        }
        counter++;
    }
    if (_showCooler) {
        if (counter == section) {
            return @"Cooler";
        }
        counter++;
    }
    if (_showFilterwheel) {
        if (counter == section) {
            return @"Filterwheel";
        }
        counter++;
    }
    if (_showFocuser) {
        if (counter == section) {
            return @"Focuser";
        }
        counter++;
    }
    if (_showGuiderport) {
        if (counter == section) {
            return @"Guiderport";
        }
        counter++;
    }
    if (_showMount) {
        if (counter == section) {
            return @"Mount";
        }
    }
    return nil;
}


@end
