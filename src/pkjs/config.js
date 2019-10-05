module.exports = [
    {
        type: 'heading',
        size: 1,
        defaultValue: 'Mile Marker',
    },
    {
        type: 'section',
        items: [
            {
                type: 'heading',
                defaultValue: 'Markers'
            },
            {
                id: 'marker-list',
                type: 'list',
            },
        ],
    },
    {
        type: 'section',
        items: [
            {
                type: 'heading',
                defaultValue: 'Export'
            },
            {
                id: 'export-type',
                type: 'select',
                defaultValue: 'json',
                options: [
                    {label: 'JSON', value: 'json'},
                    {label: 'CSV', value: 'csv'},
                ],
            },
            {
                type: 'export',
            },
        ],
    },
];
