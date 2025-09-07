classdef tRequest < matlab.unittest.TestCase
    methods(Test)
        function testRequestCreation(testCase)
            params = struct('id', '123');
            query = struct('page', '1');
            headers = struct('ContentType', 'application/json');
            data = '{"name": "test"}';
            
            req = blink.Request(params, query, headers, data);
            
            testCase.verifyEqual(req.Parameters, params);
            testCase.verifyEqual(req.Query, query);
            testCase.verifyEqual(req.Headers, headers);
            testCase.verifyEqual(req.Data, string(data));
        end
    end
end
