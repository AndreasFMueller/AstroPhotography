//
//  DriverModuleDataSource.h
//  IceStar
//
//  Created by Andreas Müller on 12.06.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "device.h"

@interface DriverModuleDataSource : NSObject<UITableViewDataSource> {
}

@property (readwrite,strong)   snowstarModulesPrx  *modules;
@property (readonly,strong) snowstarModuleNameList  *namelist;

- (id)init: (snowstarModulesPrx *) modulesprx;
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView;
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section;
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath;

@end
