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

        function testQueryStructMultipleKeys(testCase)
            query = struct('page', '2', 'sort', 'name');
            req = blink.Request(struct(), query, struct(), "");
            testCase.verifyEqual(req.Query.page, '2');
            testCase.verifyEqual(req.Query.sort, 'name');
        end

        function testQuerySanitizedFieldNamesMatchMexConvention(testCase)
            % MEX maps query keys to valid struct fields (see RequestParser::queryKeyToMatlabFieldName).
            query = struct('foo_bar', 'z');
            req = blink.Request(struct(), query, struct(), "");
            testCase.verifyEqual(req.Query.foo_bar, 'z');
        end

        function testHeadersSanitizedFieldNamesMatchMexConvention(testCase)
            % MEX maps HTTP header names to the same struct field rules as query keys.
            headers = struct('x_custom', '1');
            req = blink.Request(struct(), struct(), headers, "");
            testCase.verifyEqual(req.Headers.x_custom, '1');
        end
    end
end
