//
//  ServerTableViewDataSource.h
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "Discover.h"

@interface ServerTableViewDataSource : Discover<UITableViewDataSource>

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView;
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section;
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath;

@end
