//
//  ServiceCell.m
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "ServiceCell.h"
#include <arpa/inet.h>


@implementation ServiceCell

- (void)awakeFromNib {
    // Initialization code
}

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    NSLog(@"creating new ServiceCell");
    if (self = [super initWithStyle:style reuseIdentifier: reuseIdentifier]) {
        nameLabel = [[UILabel alloc] initWithFrame: CGRectMake(5, 5, 160, 20)];
        nameLabel.font = [UIFont systemFontOfSize: 18];
        nameLabel.textColor = [UIColor blackColor];
        [self addSubview: nameLabel];
        addressLabel = [[UILabel alloc] initWithFrame: CGRectMake(5, 27, 160, 10)];
        addressLabel.font = [UIFont systemFontOfSize: 8];
        addressLabel.textColor = [UIColor blackColor];
        [self addSubview: addressLabel];
        
        UIColor *bg = [UIColor lightGrayColor];
        
        int hoffset = 160;
        int hcapawidth = 50;
        int hoffsetstep = hcapawidth + 5;
        int voffset_lower = 26;
        int voffset_upper = 9;
        devicesLabel = [[UILabel alloc] initWithFrame: CGRectMake(hoffset, voffset_upper, hcapawidth, 10)];
        devicesLabel.backgroundColor = bg;
        devicesLabel.font = [UIFont systemFontOfSize: 7];
        devicesLabel.text = @"devices";
        devicesLabel.textAlignment = NSTextAlignmentCenter;
        [self addSubview: devicesLabel];
        
        imagesLabel = [[UILabel alloc] initWithFrame: CGRectMake(hoffset, voffset_lower, hcapawidth, 10)];
        imagesLabel.backgroundColor = bg;
        imagesLabel.font = [UIFont systemFontOfSize: 7];
        imagesLabel.text = @"images";
        imagesLabel.textAlignment = NSTextAlignmentCenter;
        [self addSubview: imagesLabel];
        
        hoffset += hoffsetstep;
        instrumentsLabel = [[UILabel alloc] initWithFrame: CGRectMake(hoffset, voffset_upper, hcapawidth, 10)];
        instrumentsLabel.backgroundColor = bg;
        instrumentsLabel.font = [UIFont systemFontOfSize: 7];
        instrumentsLabel.text = @"instruments";
        instrumentsLabel.textAlignment = NSTextAlignmentCenter;
        [self addSubview: instrumentsLabel];

        hoffset += hoffsetstep;
        guidingLabel = [[UILabel alloc] initWithFrame: CGRectMake(hoffset, voffset_upper, hcapawidth, 10)];
        guidingLabel.backgroundColor = bg;
        guidingLabel.font = [UIFont systemFontOfSize: 7];
        guidingLabel.text = @"guiding";
        guidingLabel.textAlignment = NSTextAlignmentCenter;
        [self addSubview: guidingLabel];
        
        focusingLabel = [[UILabel alloc] initWithFrame: CGRectMake(hoffset, voffset_lower, hcapawidth, 10)];
        focusingLabel.backgroundColor = bg;
        focusingLabel.font = [UIFont systemFontOfSize: 7];
        focusingLabel.text = @"focusing";
        focusingLabel.textAlignment = NSTextAlignmentCenter;
        [self addSubview: focusingLabel];
        
        hoffset += hoffsetstep;
        tasksLabel = [[UILabel alloc] initWithFrame: CGRectMake(hoffset, voffset_upper, hcapawidth, 10)];
        tasksLabel.backgroundColor = bg;
        tasksLabel.font = [UIFont systemFontOfSize: 7];
        tasksLabel.text = @"tasks";
        tasksLabel.textAlignment = NSTextAlignmentCenter;
        [self addSubview: tasksLabel];

        repositoryLabel = [[UILabel alloc] initWithFrame: CGRectMake(hoffset, voffset_lower, hcapawidth, 10)];
        repositoryLabel.backgroundColor = bg;
        repositoryLabel.font = [UIFont systemFontOfSize: 7];
        repositoryLabel.text = @"repository";
        repositoryLabel.textAlignment = NSTextAlignmentCenter;
        [self addSubview: repositoryLabel];
    
        
        [self updateState];
    }
    return self;
}

- (void)setServerinfo:(ServerInfo *)serverinfo {
    _serverinfo = serverinfo;
    nameLabel.text = serverinfo.servicename;
    [self updateState];
}

- (void)updateState {
    NSLog(@"udpate state");
    devicesLabel.hidden = !_serverinfo.devices;
    imagesLabel.hidden = !_serverinfo.images;
    instrumentsLabel.hidden = !_serverinfo.instruments;
    repositoryLabel.hidden = !_serverinfo.repository;
    guidingLabel.hidden = !_serverinfo.guiding;
    focusingLabel.hidden = !_serverinfo.focusing;
    tasksLabel.hidden = !_serverinfo.tasks;
    if (_serverinfo) {
        addressLabel.text = [NSString stringWithFormat: @"%@:%d", _serverinfo.hostname,
                            _serverinfo.port];
    }
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    if (selected) {
        devicesLabel.backgroundColor = [UIColor whiteColor];
        imagesLabel.backgroundColor = [UIColor whiteColor];
        instrumentsLabel.backgroundColor = [UIColor whiteColor];
        repositoryLabel.backgroundColor = [UIColor whiteColor];
        tasksLabel.backgroundColor = [UIColor whiteColor];
        focusingLabel.backgroundColor = [UIColor whiteColor];
        guidingLabel.backgroundColor = [UIColor whiteColor];
    } else {
        UIColor *bg = [UIColor lightGrayColor];
        devicesLabel.backgroundColor = bg;
        imagesLabel.backgroundColor = bg;
        instrumentsLabel.backgroundColor = bg;
        repositoryLabel.backgroundColor = bg;
        tasksLabel.backgroundColor = bg;
        focusingLabel.backgroundColor = bg;
        guidingLabel.backgroundColor = bg;
    }
    [super setSelected: selected animated: animated];
}

@end
