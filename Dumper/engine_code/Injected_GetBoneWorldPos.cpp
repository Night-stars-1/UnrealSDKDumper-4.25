	/**
	 * GetBoneWithRotation
	 */
	FVector USkeletalMeshComponent::GetBoneWorldPos(const int32_t& boneId) const
	{
		if (GET_BONE_MATRIX_OFFSET > 0x00)
		{
		
		   typedef void(*GetBoneMatrixType)(const USkinnedMeshComponent*, FMatrix&, int);
			static GetBoneMatrixType GetBoneMatrixPtr = nullptr;
			if (!GetBoneMatrixPtr)
				GetBoneMatrixPtr = reinterpret_cast<GetBoneMatrixType>((uintptr_t)GetModuleHandle(nullptr) + GET_BONE_MATRIX_OFFSET);
		
			FMatrix matrix;
			GetBoneMatrixPtr(this, matrix, boneId);
		
			return matrix.WPlane;
		}
		
		FVector worldPos{};
		
		const TArray<struct FTransform>& componentSpaceTransforms = CachedBoneSpaceTransforms/* CachedBoneSpaceTransforms */;
		int32_t componentToWorldOffset = 0x190 /* 0x190 || 0x1C0 || 0x1D0 || 0x250 */;
		
		if (boneId >= componentSpaceTransforms.Count())
			return worldPos;
		
		FMatrix targetBoneMatrix = componentSpaceTransforms[boneId].ToMatrixWithScale();
		FMatrix componentToWorldMatrix = (*reinterpret_cast<FTransform*>(reinterpret_cast<uintptr_t>(this) + componentToWorldOffset)).ToMatrixWithScale();
		FMatrix newMatrix = targetBoneMatrix * componentToWorldMatrix;
		
		worldPos = { newMatrix.WPlane.X, newMatrix.WPlane.Y, newMatrix.WPlane.Z };
		return worldPos;
	}
