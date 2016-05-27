//
//  ServerTableViewDataSource.m
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "ServerTableViewDataSource.h"

@implementation ServerTableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self count];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    NSInteger index = indexPath.row;
    ServerInfo  *server = [self serverAtIndex: index];
    if (nil == server) {
        return nil;
    }
    static NSString *CellIdentifier = @"ServiceListCell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    cell.textLabel.text = server.servicename;
    return cell;
}

@end
