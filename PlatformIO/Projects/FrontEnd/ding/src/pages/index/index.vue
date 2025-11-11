<template>
	<view class="content">
		<button @click="handleOpenScreen" class="btn">亮屏</button>
		
		<button class="btn">解锁</button>
		<button class="btn">复位</button>
		<button class="btn">清缓存</button>
		<button class="btn">发送回送日志</button>
		<button class="btn">叮一下</button>
	</view>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import indexReq from '@/api/index';

const title = ref('Hello');

const handleOpenScreen = async () => {
	console.log('亮屏操作');
	const data = {
		command:"click"
	};
	await indexReq
		.reqBrightScreen(data)
		.then((res:any) => {
			console.log(res)
		})
		.catch((err:any) => {
			console.log(err)
		});
};
</script>

<style lang="scss">
.content {
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	.btn{
		
		margin-bottom: 20rpx;
	}
}

.logo {
	height: 200rpx;
	width: 200rpx;
	margin-top: 200rpx;
	margin-left: auto;
	margin-right: auto;
	margin-bottom: 50rpx;
}

.text-area {
	display: flex;
	justify-content: center;
}

.title {
	font-size: 36rpx;
	color: #8f8f94;
}
</style>
