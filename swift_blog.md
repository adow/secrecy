# Optionals Case Study: valuesForKeys

This post explores how optionals help preserve strong type safety within Swift. We’re going to create a Swift version of an Objective-C API. Swift doesn’t really need this API, but it makes for a fun example.

In Objective-C, NSDictionary has a method -objectsForKeys:notFoundMarker: that takes an NSArray of keys, and returns an NSArray of corresponding values. From the documentation: “the N-th object in the returned array corresponds to the N-th key in [the input parameter] keys.” What if the third key isn’t actually in the dictionary? That’s where the notFoundMarker parameter comes in. The third element in the array will be this marker object rather than a value from the dictionary. The Foundation framework even provides a class for this case if you don’t have another to use: NSNull.

In Swift, the Dictionary type doesn’t have an objectsForKeys equivalent. For this exercise, we’re going to add one — as valuesForKeys in keeping with the common use of ‘value’ in Swift — using an extension:


	extension Dictionary {
		func valuesForKeys(keys: [K], notFoundMarker: V) -> [V] {
			// To be implemented
		}
	}

This is where our new implementation in Swift will differ from Objective-C. In Swift, the stronger typing restricts the resulting array to contain only a single type of element — we can’t put NSNull in an array of strings. However, Swift gives an even better option: we can return an array of optionals. All our values get wrapped in optionals, and instead of NSNull, we just use nil.

	extension Dictionary {
		func valuesForKeys(keys: [Key]) -> [Value?] {
			var result = [Value?]()
			result.reserveCapacity(keys.count)
			for key in keys {
				result.append(self[key])
			}
			return result
		}
	}

NOTE: Some of you may have guessed why a Swift Dictionary doesn’t need this API, and already imagined something like this:

	extension Dictionary {
		func valuesForKeys(keys: [Key]) -> [Value?] {
			return keys.map { self[$0] }
		}
	}

This has the exact same effect as the imperative version above, but all of the boilerplate has been wrapped up in the call to map. This is great example why Swift types often have a small API surface area, because it’s so easy to just call map directly.

Now we can try out some examples:

	let dict = ["A": "Amir", "B": "Bertha", "C": "Ching"]
	
	dict.valuesForKeys(["A", "C"])
	// [Optional("Amir"), Optional("Ching")]
	
	dict.valuesForKeys(["B", "D"])
	// [Optional("Bertha"), nil]
	
	dict.valuesForKeys([])
	// []
	Nested Optionals
	Now, what if we asked for the last element of each result?
	
	dict.valuesForKeys(["A", "C"]).last
	// Optional(Optional("Ching"))
	
	dict.valuesForKeys(["B", "D"]).last
	// Optional(nil)
	
	dict.valuesForKeys([]).last
	// nil

That’s strange — we have two levels of Optional in the first case, and Optional(nil) in the second case. What’s going on?

Remember the declaration of the last property:

	var last: T? { get }

This says that the last property’s type is an Optional version of the array’s element type. In this case, the element type is also optional (String?). So we end up with String??, a doubly-nested optional type.

So what does Optional(nil) mean?

Recall that in Objective-C we were going to use NSNull as a placeholder. The Objective-C version of these three calls looks like this:

	[dict valuesForKeys:@[@"A", @"C"] notFoundMarker:[NSNull null]].lastObject
	// @"Ching"
	
	[dict valuesForKeys:@[@"B", @"D"] notFoundMarker:[NSNull null]].lastObject
	// NSNull
	
	[dict valuesForKeys:@[] notFoundMarker:[NSNull null]].lastObject
	// nil

In both the Swift and Objective-C cases, a return value of nil means “the array is empty, therefore there’s no last element.” The return value of Optional(nil) (or in Objective-C NSNull) means “the last element of this array exists, but it represents an absence.” Objective-C has to rely on a placeholder object to do this, but Swift can represent it in the type system.

Providing a Default
To wrap up, what if you did want to provide a default value for anything that wasn’t in the dictionary? Well, that’s easy enough.

	extension Dictionary {
		func valuesForKeys(keys: [Key], notFoundMarker: Value) -> [Value] {
			return self.valuesForKeys(keys).map { $0 ?? notFoundMarker }
		}
	}
	
	
	dict.valuesForKeys(["B", "D"], notFoundMarker: "Anonymous")
	// ["Bertha", "Anonymous"]
	
While Objective-C has to rely on a placeholder object to do this, Swift can represent it in the type system, and provides rich syntactic support for handling optional results.