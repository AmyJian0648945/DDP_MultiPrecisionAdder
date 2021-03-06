// test vectors file
// numbers are from least significant to most significant


	uint32_t rsqModM[32] = { 0xa2cf8dc4, 0x4caefe3a, 0x8e6f2ac, 0x46c43a8a, 0xcc4d2458, 0x1ed1b354, 0x7927115a, 0xb918a54e, 0x260ae04f, 0x79ff92e2, 0x65e525b9, 0x923cd924, 0x8377e4c8, 0x5b5fa0f8, 0xc7360e04, 0x78a263ef, 0x178a9608, 0xf323cbd3, 0xad0af741, 0xf7b8ce10, 0x1a451d99, 0xc9c23ed4, 0xa0cc74f9, 0xbb09504b, 0xdcce2831, 0x9367bfbf, 0x1a23b4ec, 0xff35ee00, 0x4f92ee2a, 0x7bbaa878, 0xeee8d2aa, 0x7c4d620  };
 

    uint32_t rModM[32] = {	0x44dc949f, 0x8471f1f8, 0x2cf6a661, 0xfc628782, 0x416e25ff, 0xccd7f24e, 0xe0d5c825, 0x3e3837ac, 0xaddbdbd3, 0x430508b0, 0x83b1d6ea, 0x553872cb, 0xd0fb23a3, 0x200a0152, 0x2e5af2a2, 0x240034c, 0xc876c459, 0x3571700d, 0x3cd9abd2, 0x4502b14c, 0x6bc4555c, 0x347b7fcf, 0x70719231, 0x9db25d5a, 0x6e1529eb, 0xc90f45ff, 0xa8c4cdd8, 0x6c9dd3f7, 0xe6ba3a91, 0xb3657143, 0x2b6dbf5, 0x3d63b8f7  };


    uint32_t e = 0x101;


    uint32_t d[32] = {	0x6e4a09d1, 0x593bdd0f, 0x1902d801, 0x47a4f73e, 0x6f2dcdcd, 0x983323ba, 0x21693086, 0x1fe04c39, 0x27a789d8, 0x2df94971, 0x75624996, 0xf4f1af73, 0xf9f03713, 0x3ae7b0f0, 0xdec01705, 0x3606a04f, 0x35e6ede4, 0xac1a895a, 0xd1464fa5, 0xc8f1a063, 0xefd204f4, 0xdac9c5e5, 0x5b2e56be, 0x6053ba09, 0xebbf2804, 0x99f93e97, 0xc4f178c8, 0x619bf9c1, 0x9a33c165, 0x53cbab20, 0x1b718df7, 0xaeec1801 };


    uint32_t modullus[32] = { 0xbb236b61, 0x7b8e0e07, 0xd309599e, 0x39d787d, 0xbe91da00, 0x33280db1, 0x1f2a37da, 0xc1c7c853, 0x5224242c, 0xbcfaf74f, 0x7c4e2915, 0xaac78d34, 0x2f04dc5c, 0xdff5fead, 0xd1a50d5d, 0xfdbffcb3, 0x37893ba6, 0xca8e8ff2, 0xc326542d, 0xbafd4eb3, 0x943baaa3, 0xcb848030, 0x8f8e6dce, 0x624da2a5, 0x91ead614, 0x36f0ba00, 0x573b3227, 0x93622c08, 0x1945c56e, 0x4c9a8ebc, 0xfd49240a, 0xc29c4708  };


    uint32_t message[32] = { 0x8d27022, 0x3e2f3cc7, 0x6a66e769, 0x5d519718, 0x965bbe2f, 0x5352b15e, 0x9b3a1c1d, 0x98167f46, 0x2cead1a6, 0x551f0e20, 0xff3f0338, 0x4835fdd9, 0xf9a2dceb, 0x8ed2d0ff, 0x5cde139, 0xdb7b1f7e, 0x68abf083, 0x24a7c491, 0x282f1c77, 0x4e7ef60d, 0x9da5a9fa, 0xc3f5b1af, 0x2cf10c2c, 0x644dec24, 0x53eed310, 0x84622abb, 0x507bd3c5, 0xfaf17b14, 0x2f799929, 0x2862a26e, 0x982b1f57, 0xb0c20cfd };


    uint32_t d[32] = { 0x6e4a09d1, 0x593bdd0f, 0x1902d801, 0x47a4f73e, 0x6f2dcdcd, 0x983323ba, 0x21693086, 0x1fe04c39, 0x27a789d8, 0x2df94971, 0x75624996, 0xf4f1af73, 0xf9f03713, 0x3ae7b0f0, 0xdec01705, 0x3606a04f, 0x35e6ede4, 0xac1a895a, 0xd1464fa5, 0xc8f1a063, 0xefd204f4, 0xdac9c5e5, 0x5b2e56be, 0x6053ba09, 0xebbf2804, 0x99f93e97, 0xc4f178c8, 0x619bf9c1, 0x9a33c165, 0x53cbab20, 0x1b718df7, 0xaeec1801 }


	
	// a[32] = { a[0] ... a[32] };
	// a[32] ... a[0]
