classdef Request
    properties
        Parameters (1,1) struct
        Query (1,1) struct
        Headers (1,1) struct
        Data string {mustBeScalarOrEmpty}
    end

    methods
        function req = Request(params, query, headers, data)
            req.Parameters = params;
            req.Query = query;
            req.Headers = headers;
            req.Data = data;
        end
    end
end