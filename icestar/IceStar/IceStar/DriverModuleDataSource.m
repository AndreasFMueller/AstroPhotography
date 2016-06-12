//
//  DriverModuleDataSource.m
//  IceStar
//
//  Created by Andreas Müller on 12.06.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "DriverModuleDataSource.h"

@implementation DriverModuleDataSource

- (id)init: (snowstarModulesPrx *) modulesprx {
    if (self = [super init]) {
        self.modules = modulesprx;
        _namelist = [self.modules getModuleNames];
    }
    return self;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.modules numberOfModules];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    NSString    *modulename = [_namelist objectAtIndex: indexPath.row];
    static NSString *CellIdentifier = @"ModuleListCell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    cell.textLabel.text = modulename;
    return cell;
}

@end
