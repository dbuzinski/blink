classdef Request
    properties
        Parameters (1,1) struct
        Query (1,1) struct
        Headers (1,1) struct
        Data string {mustBeScalarOrEmpty}
    end
end