//
//  ServerTableViewDataSource.m
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "ServerTableViewDataSource.h"
#import "ServiceCell.h"

@implementation ServerTableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    NSLog(@"number of rows queried in section %ld, %ld", (long)section, (long)[self numberOfServices]);
    return [self numberOfServices];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    NSLog(@"number of sections queried");
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    NSInteger index = indexPath.row;
    NSLog(@"query cell at index %ld", (long)index);
    ServerInfo  *server = [self serverAtIndex: index];
    if (nil == server) {
        return nil;
    }
    static NSString *CellIdentifier = @"ServiceListCell";
    ServiceCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    if (nil == cell) {
        cell = [[ServiceCell alloc] init];
    }
    cell.serverinfo = server;
    return cell;
}

@end
